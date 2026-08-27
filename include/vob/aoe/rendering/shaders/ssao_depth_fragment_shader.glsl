#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssao.glsl"

in vec2 vUv;
layout(location = 0) out float oLinearDepth;


void main()
{
    // One source texel per output pixel, rotating which one so the discarded depths differ per pixel.
    // The jitter spans the source footprint, so it vanishes when source and target match.
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec2 sourceSize = vec2(textureSize(uSsao_OpaqueDepth, 0));
    vec2 footprint = max(sourceSize / vec2(uTarget.resolution) - 1.0, vec2(0.0));
    vec2 pattern = vec2(float((pixel.x + pixel.y) & 1), float(pixel.y & 1)) - 0.5;
    vec2 uv = vUv + pattern * footprint / sourceSize;

    float depth = texture(uSsao_OpaqueDepth, uv).r;
    vec4 viewH = uView.clipToView * vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

    oLinearDepth = -viewH.z / viewH.w;
}
