#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"


in vec2 vUv;
out vec4 FragColor;

float karisWeight(vec3 a_color)
{
    return 1.0 / (1.0 + dot(a_color, vec3(0.299, 0.587, 0.114)));
}

vec3 combineGroup(vec3 a_c0, vec3 a_c1, vec3 a_c2, vec3 a_c3)
{
    if (uBloom.useKarisAverage == 0)
    {
        return (a_c0 + a_c1 + a_c2 + a_c3) * 0.25;
    }

    float w0 = karisWeight(a_c0);
    float w1 = karisWeight(a_c1);
    float w2 = karisWeight(a_c2);
    float w3 = karisWeight(a_c3);
    return (a_c0 * w0 + a_c1 * w1 + a_c2 * w2 + a_c3 * w3) / (w0 + w1 + w2 + w3);
}

// 13-tap downsample, Jimenez 2014 (Call of Duty: Advanced Warfare)
void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(uBloom_Source, 0));

    vec3 a = texture(uBloom_Source, vUv + vec2(-2.0,  2.0) * texelSize).rgb;
    vec3 b = texture(uBloom_Source, vUv + vec2( 0.0,  2.0) * texelSize).rgb;
    vec3 c = texture(uBloom_Source, vUv + vec2( 2.0,  2.0) * texelSize).rgb;
    vec3 d = texture(uBloom_Source, vUv + vec2(-2.0,  0.0) * texelSize).rgb;
    vec3 e = texture(uBloom_Source, vUv).rgb;
    vec3 f = texture(uBloom_Source, vUv + vec2( 2.0,  0.0) * texelSize).rgb;
    vec3 g = texture(uBloom_Source, vUv + vec2(-2.0, -2.0) * texelSize).rgb;
    vec3 h = texture(uBloom_Source, vUv + vec2( 0.0, -2.0) * texelSize).rgb;
    vec3 i = texture(uBloom_Source, vUv + vec2( 2.0, -2.0) * texelSize).rgb;

    vec3 j = texture(uBloom_Source, vUv + vec2(-1.0,  1.0) * texelSize).rgb;
    vec3 k = texture(uBloom_Source, vUv + vec2( 1.0,  1.0) * texelSize).rgb;
    vec3 l = texture(uBloom_Source, vUv + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 m = texture(uBloom_Source, vUv + vec2( 1.0, -1.0) * texelSize).rgb;

    vec3 color = combineGroup(j, k, l, m) * 0.5
        + combineGroup(a, b, d, e) * 0.125
        + combineGroup(b, c, e, f) * 0.125
        + combineGroup(d, e, g, h) * 0.125
        + combineGroup(e, f, h, i) * 0.125;

    FragColor = vec4(max(color, 0.0), 1.0);
}
