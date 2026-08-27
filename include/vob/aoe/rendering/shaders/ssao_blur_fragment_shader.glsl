#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssao.glsl"

in vec2 vUv;
layout(location = 0) out float oAmbientOcclusion;


const int k_begin = -1;
const int k_end = 2;

void main()
{
    float centerDepth = texture(uSsao_LinearDepth, vUv).r;
    vec2 texelSize = 1.0 / vec2(textureSize(uSsao_RawOcclusion, 0));
    vec2 depthGradient = vec2(dFdx(centerDepth), dFdy(centerDepth));

    float occlusionSum = 0.0;
    float weightSum = 0.0;
    for (int y = k_begin; y <= k_end; ++y)
    {
        for (int x = k_begin; x <= k_end; ++x)
        {
            vec2 uv = vUv + vec2(x, y) * texelSize;
            float depth = texture(uSsao_LinearDepth, uv).r;

            float expectedDepth = centerDepth + dot(depthGradient, vec2(x, y));
            float weight = max(
                1.0 - abs(depth - expectedDepth) / max(centerDepth * uSsao.depthTolerance, 1e-6), 0.0);

            occlusionSum += texture(uSsao_RawOcclusion, uv).r * weight;
            weightSum += weight;
        }
    }

    oAmbientOcclusion = weightSum > 0.0 ? occlusionSum / weightSum : texture(uSsao_RawOcclusion, vUv).r;
}
