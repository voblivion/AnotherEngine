#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"


in vec2 vUv;
out vec4 FragColor;

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 tonemapAces(vec3 color)
{
    return clamp((color * (2.51 * color + 0.03)) / (color * (2.43 * color + 0.59) + 0.14), 0.0, 1.0);
}

vec3 encodeSrgb(vec3 color)
{
    vec3 low = color * 12.92;
    vec3 high = 1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055;
    return mix(high, low, lessThanEqual(color, vec3(0.0031308)));
}

void main()
{
    vec3 color = texture(uTonemap_Color, vUv).rgb;

    color *= uTonemap.exposure * uTonemap.colorFilter;
    color = encodeSrgb(tonemapAces(color));

    float luma = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(luma), color, uTonemap.saturation);
    color = clamp((color - 0.5) * uTonemap.contrast + 0.5, 0.0, 1.0);

    FragColor = vec4(color, 1.0);
}
