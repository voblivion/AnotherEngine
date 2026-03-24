#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_utils.glsl"

in vec2 vUv;
out vec4 oSsrColor;


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
        oSsrColor = vec4(0.0, 0.0, 0.5, 0.0);
        return;
    }

    // --- reconstruct view-space position and normal ---
    vec3 viewPos = ReconstructViewPos2(vUv, depth);
    vec3 normal  = normalize(mat3(uView.worldToView) * texture(uSsr_OpaqueNormal, vUv).xyz);

    // --- reflection ray in view space ---
    vec3 incident = normalize(viewPos - vec3(0.0)); // view-space: camera is at origin
    vec3 reflDir  = reflect(incident, normal);
    
    // --- project a far point along reflDir to get NDC ray direction ---
    
    int steps = 1 << uSsr.log2Step;
    float maxRange = uSsr.maxRangeRatio * steps;
    
    vec4 reflEndClip = uView.viewToClip * vec4(viewPos + reflDir * maxRange, 1.0);
    vec3 reflEndNDC  = reflEndClip.xyz / reflEndClip.w;

    // avoid self hits?
    float bias = LinearizeDepth(depth) * uSsr.initialBiasRatio;
    vec3 biasedViewPos = viewPos + normal * bias;

    vec4 startClip = uView.viewToClip * vec4(biasedViewPos, 1.0);
    vec3 startNDC  = startClip.xyz / startClip.w;

    // step in NDC space, convert to UV as we go
    vec3 rayStepNDC = (reflEndNDC - startNDC) / float(steps);
    
    
    // no hit
    vec4 color = vec4(0.0, 0.0, 0.0, 0);
    vec3 sampleNDC = startNDC + rayStepNDC; // start one step ahead to avoid self-hit

    for (int i = 0; i < steps; ++i, sampleNDC += rayStepNDC)
    {
        vec2 uv = sampleNDC.xy * 0.5 + 0.5;

        // out of screen
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) break;

        float sceneDepth = texture(uSsr_OpaqueDepth, uv).r;
        float linearScene = LinearizeDepth(sceneDepth);
        float linearRay   = LinearizeDepth(sampleNDC.z * 0.5 + 0.5);

        float diff = linearRay - linearScene;
        float thickness = min(linearScene * uSsr.thicknessRatio, uSsr.maxThickness);
        if (diff > 0.0 && diff < linearScene * thickness)
        {
            color = vec4(texture(uSsr_DirectOpaqueColor, uv).rgb, 1.0);
            
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
            color = vec4(texture(uSsr_DirectOpaqueColor, sampleNDC.xy * 0.5 + 0.5).rgb, 1.0);
            break;
        }
    }
    oSsrColor = color;
}
