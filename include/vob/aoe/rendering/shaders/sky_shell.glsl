#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"

in vec2 vUv;
out vec4 oColor;

vec3 getSkyColor(vec3 dir, float lod);

vec3 uReconstructWorldDir(vec2 uv)
{
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, 1.0, 1.0);
    vec4 viewPos = uView.clipToView * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = uView.viewToWorld * vec4(viewPos.xyz, 0.0);
    return normalize(worldPos.xyz);
}

void main()
{
    oColor = vec4(getSkyColor(uReconstructWorldDir(vUv), 0.0), 1.0);
}
