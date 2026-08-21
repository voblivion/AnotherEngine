#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssr.glsl"
#include "core/light_utils.glsl"

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

// irradiance is a cosine-weighted integral, so it needs dividing by PI to read as radiance
vec3 skyIrradianceRadiance(vec3 direction)
{
    return max(uEvaluateSkyIrradiance(direction), vec3(0.0)) / 3.14159265358979;
}

void main()
{
    // --- skip background ---
    float depth = textureLod(uSsr_OpaqueDepth, vUv, 0.0).r;
    if (1.0 - depth <= 0.0001)
    {
        oSsrColor = vec4(0.0);
        return;
    }

    vec4 surface = textureLod(uSsr_OpaqueSurface, vUv, 0.0);
    if (surface.r == 0.0)
    {
        oSsrColor = vec4(0.0);
        return;
    }

    // --- reconstruct view-space position and normal ---
    vec3 viewPos = ReconstructViewPos2(vUv, depth);
    vec3 normal  = normalize(mat3(uView.worldToView) * textureLod(uSsr_OpaqueNormal, vUv, 0.0).xyz);

    // --- reflection ray in view space ---
    vec3 incident = normalize(viewPos - vec3(0.0)); // view-space: camera is at origin
    vec3 reflDir  = reflect(incident, normal);

    float roughness = surface.a;
    float NdotV = max(dot(normal, -incident), 0.0);
    vec2 envBrdf = envBrdfApprox(NdotV, roughness);
    vec3 specularWeight = surface.rgb * envBrdf.x + envBrdf.y;

    const float k_minSpecularWeight = 0.005;
    if (max(max(specularWeight.r, specularWeight.g), specularWeight.b) < k_minSpecularWeight)
    {
        oSsrColor = vec4(0.0);
        return;
    }

    // rays travelling back toward the camera need information the screen does not hold, and one
    // mirror ray only represents a narrow lobe - the wider it gets, the less a single sample says
    // about it, so lean on the sky's prefiltered version instead
    float screenTrust = smoothstep(0.25, 0.0, reflDir.z) * (1.0 - smoothstep(0.45, 0.85, roughness));

    vec3 reflDirWorld = normalize(mat3(uView.viewToWorld) * reflDir);

    // both factors are known before marching, so nothing the march finds could change the answer
    if (screenTrust <= 0.0)
    {
        oSsrColor = vec4(skyIrradianceRadiance(reflDirWorld), 1.0);
        return;
    }

    // at the roughest end the lobe is wide enough that the SH hemisphere stands in for the sky, at
    // nine madds instead of a procedural sky evaluation
    float skyBlend = smoothstep(0.45, 0.85, roughness);
    vec3 skyColor = getSkyColor(reflDirWorld, clamp(roughness * 2.0, 0.0, 1.0));
    if (skyBlend > 0.0)
    {
        skyColor = mix(skyColor, skyIrradianceRadiance(reflDirWorld), skyBlend);
    }

    // --- project a far point along reflDir to get NDC ray direction ---

    int maxSteps = 1 << uSsr.log2Step;

    // a ray reaching past the near plane projects through a negative w and lands nowhere useful, so
    // cut it at the near plane first
    vec3 endViewPos = viewPos + reflDir * uSsr.maxRange;
    if (endViewPos.z > -uView.nearClip)
    {
        float nearT = (-uView.nearClip - viewPos.z) / (endViewPos.z - viewPos.z);
        endViewPos = viewPos + (endViewPos - viewPos) * clamp(nearT, 0.0, 1.0);
    }

    vec4 reflEndClip = uView.viewToClip * vec4(endViewPos, 1.0);
    vec3 reflEndNDC  = reflEndClip.xyz / reflEndClip.w;

    // avoid self hits?
    float originLinear = LinearizeDepth(depth);
    float bias = originLinear * uSsr.initialBiasRatio;
    vec3 biasedViewPos = viewPos + normal * bias;

    vec4 startClip = uView.viewToClip * vec4(biasedViewPos, 1.0);
    vec3 startNDC  = startClip.xyz / startClip.w;

    // one step per pixel of the longer screen axis: nothing samples a texel twice, and log2Step
    // becomes a budget for long rays rather than a fixed subdivision of any ray
    vec2 startPixel = (startNDC.xy * 0.5 + 0.5) * vec2(uTarget.resolution);
    vec2 endPixel = (reflEndNDC.xy * 0.5 + 0.5) * vec2(uTarget.resolution);
    vec2 pixelSpan = abs(endPixel - startPixel);
    float stepCount = clamp(max(pixelSpan.x, pixelSpan.y), 1.0, float(maxSteps));

    // step in NDC space, convert to UV as we go
    vec3 rayStepNDC = (reflEndNDC - startNDC) / stepCount;
    
    
    vec3 hitColor = vec3(0.0);
    float confidence = 0.0;

    // ambiguous blockers accumulate front to back: each one absorbs part of the ray, and whatever
    // transmittance survives is what still reaches a real hit or the sky
    vec3 blockedColor = vec3(0.0);
    float transmittance = 1.0;

    vec3 sampleNDC = startNDC + rayStepNDC; // start one step ahead to avoid self-hit

    float prevRay = LinearizeDepth(startNDC.z * 0.5 + 0.5);
    float prevDiff = -1.0; // the ray starts in front of the surface it left

    for (int i = 0; i < int(stepCount); ++i, sampleNDC += rayStepNDC)
    {
        vec2 uv = sampleNDC.xy * 0.5 + 0.5;

        // out of screen
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) break;

        float sceneDepth = textureLod(uSsr_OpaqueDepth, uv, 0.0).r;
        float linearScene = LinearizeDepth(sceneDepth);
        float linearRay   = LinearizeDepth(sampleNDC.z * 0.5 + 0.5);

        float diff = linearRay - linearScene;
        float advance = max(linearRay - prevRay, 0.0);

        bool crossed = prevDiff <= 0.0 && diff > 0.0;
        prevRay = linearRay;
        prevDiff = diff;

        // while the ray still hugs the surface it left, a crossing is its own geometry
        bool separated = (linearRay - originLinear) > uSsr.minSeparationRatio * originLinear;
        float thickness = max(advance, min(linearScene * uSsr.thicknessRatio, uSsr.maxThickness));
        float blockedness = diff / max(thickness, 0.0001);
        if (!crossed || !separated || blockedness >= uSsr.blockedSkyThickness)
        {
            continue;
        }

        vec3 crossingNDC = sampleNDC - rayStepNDC; // last known miss
        vec3 bisectStep = rayStepNDC * 0.5;

        int subStep = 1 << uSsr.log2SubStep;
        for (int j = 0; j < subStep; ++j)
        {
            crossingNDC += bisectStep;
            vec2 uvMid = crossingNDC.xy * 0.5 + 0.5;

            float midDepth = textureLod(uSsr_OpaqueDepth, uvMid, 0.0).r;
            float midLinear = LinearizeDepth(midDepth);
            float midRayLinear = LinearizeDepth(crossingNDC.z * 0.5 + 0.5);

            bisectStep *= 0.5;
            if (midRayLinear - midLinear > 0.0)
                crossingNDC -= bisectStep; // behind surface, step back
        }

        vec2 hitUv = crossingNDC.xy * 0.5 + 0.5;
        float hitScene = LinearizeDepth(textureLod(uSsr_OpaqueDepth, hitUv, 0.0).r);
        float hitDiff = LinearizeDepth(crossingNDC.z * 0.5 + 0.5) - hitScene;
        float hitThickness = max(advance, min(hitScene * uSsr.thicknessRatio, uSsr.maxThickness));
        vec3 candidateColor = textureLod(uSsr_DirectOpaqueColor, hitUv, 0.0).rgb;

        if (hitDiff < hitThickness)
        {
            hitColor = candidateColor;

            vec2 borderDist = min(hitUv, 1.0 - hitUv);
            confidence = smoothstep(0.0, 0.1, min(borderDist.x, borderDist.y));
            break;
        }

        // how far the ray plunged past this thing is a coarse-sample question; the refined
        // crossing sits at diff ~ 0 by construction and says nothing about it
        float ratio = smoothstep(uSsr.blockedBlackThickness, uSsr.blockedSkyThickness, blockedness);
        blockedColor += transmittance * (1.0 - ratio) * candidateColor;
        transmittance *= ratio;

        if (transmittance < 0.1)
        {
            break;
        }
    }

    // everything the march produced - blockers included - is screen-space, so all of it answers to
    // the same trust; without this, blocked colour reaches rough surfaces the fade excludes
    vec3 screenColor = blockedColor + transmittance * mix(skyColor, hitColor, confidence);

    oSsrColor = vec4(mix(skyColor, screenColor, screenTrust), 1.0);
}
