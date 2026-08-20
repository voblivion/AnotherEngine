#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssr_filter.glsl"
#include "core/math_utils.glsl"


in vec2 vUv;
out vec4 oColor;

float karisWeight(vec3 a_color)
{
    return 1.0 / (1.0 + dot(a_color, vec3(0.299, 0.587, 0.114)));
}

void main()
{
    vec2 texelSize = uTarget.invResolution;

    vec3 centerNormal = texture(uSsrFilter_OpaqueNormal, vUv).xyz;
    float centerDepth = linearizeDepth(
        texture(uSsrFilter_OpaqueDepth, vUv).r, uView.nearClip, uView.farClip);

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;
    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            vec2 tapUv = vUv + vec2(x, y) * texelSize;
            vec3 tap = texture(uSsrFilter_Source, tapUv).rgb;

            vec3 tapNormal = texture(uSsrFilter_OpaqueNormal, tapUv).xyz;
            float tapDepth = linearizeDepth(
                texture(uSsrFilter_OpaqueDepth, tapUv).r, uView.nearClip, uView.farClip);

            float normalWeight = pow(max(dot(centerNormal, tapNormal), 0.0), 32.0);
            float depthWeight = exp(-abs(tapDepth - centerDepth) / max(0.02 * centerDepth, 0.001));

            float weight = karisWeight(tap) * normalWeight * depthWeight;
            color += tap * weight;
            totalWeight += weight;
        }
    }

    oColor = vec4(totalWeight > 0.0 ? max(color / totalWeight, 0.0) : vec3(0.0), 1.0);
}
