#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_utils.glsl"

in vec2 vUv;
out vec4 oColor;


void main()
{
    float roughness = texture(uOpaqueComposition_OpaqueSurface, vUv).a;
    float maxLod = float(textureQueryLevels(uOpaqueComposition_SsrColor) - 1);
    vec3 reflection = textureLod(uOpaqueComposition_SsrColor, vUv, roughness * maxLod).rgb;

    oColor = vec4(texture(uOpaqueComposition_DirectOpaqueColor, vUv).rgb + reflection, 1.0);
}
