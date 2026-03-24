#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/math_utils.glsl"

in vec2 vUv;
layout(location = 0) out float oAmbientOcclusion;


// void main (void)
// {
    // vec4 rndTable [8] = vec4 [8]
    // (
        // vec4 ( -0.5, -0.5, -0.5, 0.0 ),
        // vec4 ( 0.5, -0.5, -0.5, 0.0 ),
        // vec4 ( -0.5, 0.5, -0.5, 0.0 ),
        // vec4 ( 0.5, 0.5, -0.5, 0.0 ),
        // vec4 ( -0.5, -0.5, 0.5, 0.0 ),
        // vec4 ( 0.5, -0.5, 0.5, 0.0 ),
        // vec4 ( -0.5, 0.5, 0.5, 0.0 ),
        // vec4 ( 0.5, 0.5, 0.5, 0.0 )
    // );
    // vec3 normal = normalize(texture(uSsao_OpaqueNormal, vUv).rgb); // assuming stored in world space
    // vec3 normalView = normalize(mat3(uView.worldToView) * normal);
    // float depth = texture2D (uSsao_OpaqueDepth, vUv).r;
    // float z = uView.farClip * uView.nearClip / (depth * (uView.farClip - uView.nearClip) - uView.farClip);
    // float attenuation = 0.0;

    // for (int i = 0; i < 8; i++)
    // {
        // vec3 samp = reflect(rndTable[i].xyz, normalView);
        // float sampDepth = texture2D (uSsao_OpaqueDepth, vUv + uSsao.radius * samp.xy / z ).r;
        // float sampZ = uView.farClip * uView.nearClip / (sampDepth * (uView.farClip - uView.nearClip) - uView.farClip);

        // if (sampZ - z > 0.1)
            // continue;

        // float dz = max(sampZ - z, 0.0) * 33.0;

        // attenuation += 1.0 / (1.0 + dz * dz);
    // }

    // attenuation = clamp((attenuation / 8.0 + uSsao.attenuationBias) * uSsao.attenuationScale, 0.0, 1.0);
    
    // oColor = vec4(texture(uSsao_DirectOpaqueColor, vUv).rgb * attenuation, 1.0);
// }
float noise(vec2 uv)
{
    return fract(52.9829189 * fract(dot(uv, vec2(0.06711056, 0.00583715))));
}

void main()
{
    vec3 normalWorld = texture(uSsao_OpaqueGeometricNormal, vUv).rgb;
    vec3 N = normalize(mat3(uView.worldToView) * normalWorld);
    float depth = texture(uSsao_OpaqueDepth, vUv).r;
    float z = uView.farClip * uView.nearClip / (depth * (uView.farClip - uView.nearClip) - uView.farClip);
    vec2 ndc = vUv * 2.0 - 1.0;
    vec4 viewDirH = uView.clipToView * vec4(ndc, depth * 2.0 - 1.0, 1.0);
    vec3 viewDir = normalize(viewDirH.xyz / viewDirH.w);
    vec3 dir = reflect(viewDir, N);
    vec3 left = cross(N, dir);
    vec2 texelSize = 1.0 / textureSize(uSsao_OpaqueDepth, 0);
    
    float attenuation = 0.0;
    int sampleLayerCount = uSsao.sampleCount >> 1;
    for (int i = 0; i < sampleLayerCount; ++i)
    {
        for (int j = -i; j <= i; ++j)
        {
            vec3 sampDir = i == 0 ? dir : (cos(float(j) / i) * dir + sin(float(j) / i) * left);
            float sampDepth = texture(uSsao_OpaqueDepth, vUv + sampDir.xy * texelSize * uSsao.radius * float(i) / sampleLayerCount).r;
            float sampZ = uView.farClip * uView.nearClip / (sampDepth * (uView.farClip - uView.nearClip) - uView.farClip);
            
            if (sampZ - z > uSsao.threshold || sampZ - z < 0.0)
            {
                float dz = max(sampZ - z, 0.0) * 33.0;
                attenuation += 1.0;
            }
            
        }
    }
    
    attenuation = clamp((attenuation / float(uSsao.sampleCount * uSsao.sampleCount) + uSsao.attenuationBias) * uSsao.attenuationScale, 0.0, 1.0);
    oAmbientOcclusion = attenuation;
}

// void main()
// {
    // vec3 normalWorld = normalize(texture(uSsao_OpaqueGeometricNormal, vUv).rgb);
    // vec3 N = normalize(mat3(uView.worldToView) * normalWorld);

    // float depth = texture(uSsao_OpaqueDepth, vUv).r;
    // float z = uView.farClip * uView.nearClip / (depth * (uView.farClip - uView.nearClip) - uView.farClip);

    // // Build TBN to orient hemisphere around normal
    // vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    // vec3 T = normalize(cross(up, N));
    // vec3 B = cross(N, T);

    // // Per-pixel random rotation angle
    // float angle = noise(gl_FragCoord.xy) * 6.28318;
    // float cosA = cos(angle);
    // float sinA = sin(angle);

    // float attenuation = 0.0;

    // for (int i = 0; i < uSsao.sampleCount; i++)
    // {
        // // Fibonacci hemisphere distribution
        // float fi = float(i) + 0.5;
        // float phi = acos(1.0 - fi / float(uSsao.sampleCount));
        // float theta = 2.39996 * fi; // golden angle

        // // Sample direction in hemisphere around N
        // vec3 samp = sin(phi) * (cos(theta) * T + sin(theta) * B) + cos(phi) * N;

        // // Rotate by per-pixel noise
        // vec2 rotatedXY = vec2(
            // cosA * samp.x - sinA * samp.y,
            // sinA * samp.x + cosA * samp.y
        // );
        // samp.xy = rotatedXY;

        // float sampDepth = texture(uSsao_OpaqueDepth, vUv + uSsao.radius * samp.xy / z).r;
        // float sampZ = uView.farClip * uView.nearClip / (sampDepth * (uView.farClip - uView.nearClip) - uView.farClip);

        // if (sampZ - z > 0.1)
            // continue;

        // float dz = max(sampZ - z, 0.0) * 33.0;
        // attenuation += 1.0 / (1.0 + dz * dz);
    // }

    // attenuation = clamp((attenuation / float(uSsao.sampleCount) + uSsao.attenuationBias) * uSsao.attenuationScale, 0.0, 1.0);
    // oColor = vec4(texture(uSsao_DirectOpaqueColor, vUv).rgb * attenuation, 1.0);
// }