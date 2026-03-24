#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_utils.glsl"

in vec2 vUv;
out vec4 oColor;


void main()
{
    oColor = texture(uOpaqueComposition_DirectOpaqueColor, vUv)
        + texture(uOpaqueComposition_SsrColor, vUv) * texture(uOpaqueComposition_OpaqueSurface, vUv).r;
}
