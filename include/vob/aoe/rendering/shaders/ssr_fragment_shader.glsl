#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_utils.glsl"

in vec2 vUv;
out vec4 oSsrColor;

vec3 getSkyColor(vec3 dir, float lod);


vec3 ReconstructViewPos2(vec2 uv, float depth)
{
    float linearDepth = uView.nearClip * uView.farClip / (uView.farClip - depth * (uView.farClip - uView.nearClip));
    
    float tanHalfFovX = 1.0 / uView.viewToClip[0][0];
    float tanHalfFovY = 1.0 / uView.viewToClip[1][1];
    
    vec2 ndc = uv * 2.0 - 1.0;
    float viewZ = -linearDepth;
    float viewX = ndc.x * tanHalfFovX * linearDepth;
    float viewY = ndc.y * tanHalfFovY * linearDepth;
    
    return vec3(viewX, viewY, viewZ);
}

float LinearizeDepth(float depth)
{
    return uView.nearClip * uView.farClip / (uView.farClip - depth * (uView.farClip - uView.nearClip));
}

// Lazarov's analytic fit of the split-sum environment BRDF, so no LUT is needed
vec2 envBrdfApprox(float NdotV, float roughness)
{
    const vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    const vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28 * NdotV)) * r.x + r.y;
    return vec2(-1.04, 1.04) * a004 + r.zw;
}

void main()
{
    // --- skip background ---
    float depth = texture(uSsr_OpaqueDepth, vUv).r;
    if (1.0 - depth <= 0.0001)
    {
        oSsrColor = vec4(0.0);
        return;
    }

    vec3 surface = texture(uSsr_OpaqueSurface, vUv).rgb;
    if (surface.r == 0.0)
    {
        oSsrColor = vec4(0.0);
        return;
    }

    // --- reconstruct view-space position and normal ---
    vec3 viewPos = ReconstructViewPos2(vUv, depth);
    vec3 normal  = normalize(mat3(uView.worldToView) * texture(uSsr_OpaqueNormal, vUv).xyz);

    // --- reflection ray in view space ---
    vec3 incident = normalize(viewPos - vec3(0.0)); // view-space: camera is at origin
    vec3 reflDir  = reflect(incident, normal);

    float roughness = texture(uSsr_OpaqueSurface, vUv).a;
    float NdotV = max(dot(normal, -incident), 0.0);
    vec2 envBrdf = envBrdfApprox(NdotV, roughness);
    vec3 specularWeight = surface * envBrdf.x + envBrdf.y;

    const float k_minSpecularWeight = 0.005;
    if (max(max(specularWeight.r, specularWeight.g), specularWeight.b) < k_minSpecularWeight)
    {
        oSsrColor = vec4(0.0);
        return;
    }

    // --- project a far point along reflDir to get NDC ray direction ---

    int steps = 1 << uSsr.log2Step;

    vec4 reflEndClip = uView.viewToClip * vec4(viewPos + reflDir * uSsr.maxRange, 1.0);
    vec3 reflEndNDC  = reflEndClip.xyz / reflEndClip.w;

    // avoid self hits?
    float bias = LinearizeDepth(depth) * uSsr.initialBiasRatio;
    vec3 biasedViewPos = viewPos + normal * bias;

    vec4 startClip = uView.viewToClip * vec4(biasedViewPos, 1.0);
    vec3 startNDC  = startClip.xyz / startClip.w;

    // step in NDC space, convert to UV as we go
    vec3 rayStepNDC = (reflEndNDC - startNDC) / float(steps);
    
    
    vec3 hitColor = vec3(0.0);
    float confidence = 0.0;
    vec3 sampleNDC = startNDC + rayStepNDC; // start one step ahead to avoid self-hit

    float prevRay = LinearizeDepth(startNDC.z * 0.5 + 0.5);
    float prevDiff = -1.0; // the ray starts in front of the surface it left

    for (int i = 0; i < steps; ++i, sampleNDC += rayStepNDC)
    {
        vec2 uv = sampleNDC.xy * 0.5 + 0.5;

        // out of screen
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) break;

        float sceneDepth = texture(uSsr_OpaqueDepth, uv).r;
        float linearScene = LinearizeDepth(sceneDepth);
        float linearRay   = LinearizeDepth(sampleNDC.z * 0.5 + 0.5);

        float diff = linearRay - linearScene;
        float advance = max(linearRay - prevRay, 0.0);
        float thickness = max(advance, min(linearScene * uSsr.thicknessRatio, uSsr.maxThickness));

        bool crossed = prevDiff <= 0.0 && diff > 0.0;
        prevRay = linearRay;
        prevDiff = diff;

        if (crossed && diff < thickness)
        {
            vec3 bisectStep = rayStepNDC * 0.5;
            sampleNDC -= rayStepNDC; // go back to last known miss
            
            int subStep = 1 << uSsr.log2SubStep;
            for (int j = 0; j < subStep; ++j)
            {
                sampleNDC += bisectStep;
                vec2 uvMid = sampleNDC.xy * 0.5 + 0.5;

                float midDepth = texture(uSsr_OpaqueDepth, uvMid).r;
                float midLinear = LinearizeDepth(midDepth);
                float midRayLinear = LinearizeDepth(sampleNDC.z * 0.5 + 0.5);

                bisectStep *= 0.5;
                if (midRayLinear - midLinear > 0.0)
                    sampleNDC -= bisectStep; // behind surface, step back
            }
            vec2 hitUv = sampleNDC.xy * 0.5 + 0.5;
            hitColor = texture(uSsr_DirectOpaqueColor, hitUv).rgb;

            vec2 borderDist = min(hitUv, 1.0 - hitUv);
            confidence = smoothstep(0.0, 0.1, min(borderDist.x, borderDist.y));
            break;
        }
    }

    // rays travelling back toward the camera need information the screen does not hold
    confidence *= smoothstep(0.25, 0.0, reflDir.z);

    float skyLod = clamp(roughness * 2.0, 0.0, 1.0);
    vec3 reflDirWorld = normalize(mat3(uView.viewToWorld) * reflDir);
    vec3 radiance = mix(getSkyColor(reflDirWorld, skyLod), hitColor, confidence);

    oSsrColor = vec4(radiance * specularWeight, 1.0);
}
