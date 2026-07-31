#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"

in vec2 vUv;
out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(uPresent_Color, vUv).rgb, 1.0);
}
