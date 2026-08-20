#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssr_filter.glsl"


in vec2 vUv;
out vec4 oColor;

float karisWeight(vec3 a_color)
{
    return 1.0 / (1.0 + dot(a_color, vec3(0.299, 0.587, 0.114)));
}

vec3 combineGroup(vec3 a_c0, vec3 a_c1, vec3 a_c2, vec3 a_c3)
{
    float w0 = karisWeight(a_c0);
    float w1 = karisWeight(a_c1);
    float w2 = karisWeight(a_c2);
    float w3 = karisWeight(a_c3);
    return (a_c0 * w0 + a_c1 * w1 + a_c2 * w2 + a_c3 * w3) / (w0 + w1 + w2 + w3);
}

void main()
{
    vec2 texelSize = uTarget.invResolution * 0.5;

    vec3 a = texture(uSsrFilter_Source, vUv + vec2(-2.0,  2.0) * texelSize).rgb;
    vec3 b = texture(uSsrFilter_Source, vUv + vec2( 0.0,  2.0) * texelSize).rgb;
    vec3 c = texture(uSsrFilter_Source, vUv + vec2( 2.0,  2.0) * texelSize).rgb;
    vec3 d = texture(uSsrFilter_Source, vUv + vec2(-2.0,  0.0) * texelSize).rgb;
    vec3 e = texture(uSsrFilter_Source, vUv).rgb;
    vec3 f = texture(uSsrFilter_Source, vUv + vec2( 2.0,  0.0) * texelSize).rgb;
    vec3 g = texture(uSsrFilter_Source, vUv + vec2(-2.0, -2.0) * texelSize).rgb;
    vec3 h = texture(uSsrFilter_Source, vUv + vec2( 0.0, -2.0) * texelSize).rgb;
    vec3 i = texture(uSsrFilter_Source, vUv + vec2( 2.0, -2.0) * texelSize).rgb;

    vec3 j = texture(uSsrFilter_Source, vUv + vec2(-1.0,  1.0) * texelSize).rgb;
    vec3 k = texture(uSsrFilter_Source, vUv + vec2( 1.0,  1.0) * texelSize).rgb;
    vec3 l = texture(uSsrFilter_Source, vUv + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 m = texture(uSsrFilter_Source, vUv + vec2( 1.0, -1.0) * texelSize).rgb;

    vec3 color = combineGroup(j, k, l, m) * 0.5
        + combineGroup(a, b, d, e) * 0.125
        + combineGroup(b, c, e, f) * 0.125
        + combineGroup(d, e, g, h) * 0.125
        + combineGroup(e, f, h, i) * 0.125;

    oColor = vec4(max(color, 0.0), 1.0);
}
