#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_outputs.glsl"

in vec3 vPosition;
in vec2 vUv;
in mat3 vTBN;

layout(location = 0) out vec3 oColor;
layout(location = 1) out vec3 oNormal;
layout(location = 2) out vec4 oSurface;

OpaqueOutputs getOpaqueOutputs();

void main()
{
    OpaqueOutputs outputs = getOpaqueOutputs();

    oColor = outputs.color;
    oNormal = outputs.normal;
    oSurface = outputs.surface;
}
