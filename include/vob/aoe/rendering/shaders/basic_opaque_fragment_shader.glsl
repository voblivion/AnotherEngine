#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_utils.glsl"

layout(std140, binding = BINDING_UBO_MATERIAL) uniform BasicOpaqueParams
{
    vec4 uAlbedo;
    float uMetallic;
    float uRoughness;
};

in vec3 vPosition;
in vec2 vUv;
in mat3 vTBN;

layout(location = 0) out vec3 oColor;
layout(location = 1) out vec3 oNormal;
layout(location = 2) out vec3 oSurface;

void main()
{
    vec3 normal = normalize(vTBN * vec3(0.0, 0.0, 1.0));
    oColor = uEvaluateLights(gl_FragCoord, vPosition, normal, uAlbedo.xyz, uMetallic, uRoughness, 0.0);
    oNormal = normal;
    oSurface = vec3(uMetallic, uRoughness, 0.0);
}
