#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"


in vec2 vUv;
out vec4 FragColor;

void main()
{
    vec3 scene = texture(uBloomCombine_Scene, vUv).rgb;
    vec3 bloom = texture(uBloomCombine_Bloom, vUv).rgb;

    FragColor = vec4(mix(scene, bloom / uBloom.totalWeight, uBloom.strength), 1.0);
}
