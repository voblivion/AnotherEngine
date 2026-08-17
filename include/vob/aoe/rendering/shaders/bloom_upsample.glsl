#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_bloom.glsl"


in vec2 vUv;
out vec4 FragColor;

// 9-tap tent, 1 2 1 / 2 4 2 / 1 2 1 over 16. Accumulation is the blend, not this.
void main()
{
    vec2 offset = uBloom.filterRadius / vec2(textureSize(uBloom_Source, 0));

    vec3 a = texture(uBloom_Source, vUv + vec2(-1.0,  1.0) * offset).rgb;
    vec3 b = texture(uBloom_Source, vUv + vec2( 0.0,  1.0) * offset).rgb;
    vec3 c = texture(uBloom_Source, vUv + vec2( 1.0,  1.0) * offset).rgb;
    vec3 d = texture(uBloom_Source, vUv + vec2(-1.0,  0.0) * offset).rgb;
    vec3 e = texture(uBloom_Source, vUv).rgb;
    vec3 f = texture(uBloom_Source, vUv + vec2( 1.0,  0.0) * offset).rgb;
    vec3 g = texture(uBloom_Source, vUv + vec2(-1.0, -1.0) * offset).rgb;
    vec3 h = texture(uBloom_Source, vUv + vec2( 0.0, -1.0) * offset).rgb;
    vec3 i = texture(uBloom_Source, vUv + vec2( 1.0, -1.0) * offset).rgb;

    vec3 color = (a + c + g + i)
        + (b + d + f + h) * 2.0
        + e * 4.0;

    FragColor = vec4(uBloom.scatter * color * (1.0 / 16.0), 1.0);
}
