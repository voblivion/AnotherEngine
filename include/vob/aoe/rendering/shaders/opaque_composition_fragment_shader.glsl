#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_utils.glsl"

in vec2 vUv;
out vec4 oColor;


void main()
{
    vec3 f0 = texture(uOpaqueComposition_OpaqueSurface, vUv).rgb;
    oColor = vec4(
        texture(uOpaqueComposition_DirectOpaqueColor, vUv).rgb + texture(uOpaqueComposition_SsrColor, vUv).rgb * f0, 1.0);
}
