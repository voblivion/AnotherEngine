#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/shading_utils.glsl"

layout(binding = BINDING_TEXTURE_MATERIAL_CUSTOMS_BEGIN) uniform sampler2D albedo;
layout(binding = BINDING_TEXTURE_MATERIAL_CUSTOMS_BEGIN + 1) uniform sampler2D normalMap;
layout(binding = BINDING_TEXTURE_MATERIAL_CUSTOMS_BEGIN + 2) uniform sampler2D metallicRoughness;

in vec3 vPosition;
in vec2 vUv;
in mat3 vTBN;

layout(location = 0) out vec3 oColor;
layout(location = 1) out vec3 oNormal;
layout(location = 2) out vec3 oSurface;

void main()
{
    vec3 albedoSample = texture(albedo, vUv).xyz;
    vec2 metallicRoughnessSample = texture(metallicRoughness, vUv).rg;
    vec3 normalMapSample = texture(normalMap, vUv).rgb * 2.0 - 1.0;
    vec3 normal = normalize(vTBN * normalMapSample);
    
    oColor = uEvaluateLights(gl_FragCoord, vPosition, normal, albedoSample, metallicRoughnessSample.r, metallicRoughnessSample.g, 0.0);
    oNormal = normal;
    oSurface = vec3(metallicRoughnessSample, 0.0);
}
