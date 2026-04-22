#ifndef VOB_AOEGL_SHADING_UTILS_GLSL
#define VOB_AOEGL_SHADING_UTILS_GLSL

#include "core/light_utils.glsl"

int uComputeLightClusterIndex()
{
    return computeLightClusterIndex(
        gl_FragCoord.xy,
        gl_FragCoord.z,
        uLighting.lightClusterTileSize,
        uLighting.lightClusterZCount,
        uView.resolution,
        uView.nearClip,
        uView.farClip);
}

float uEvaluateShadow(int lightIndex, vec3 position, vec3 normal)
{
    if (uLights[lightIndex].shadowMapIndex < 0)
    {
        return 0.0;
    }
    
    vec3 lightPosition = uLights[lightIndex].position;
    int spotLightShadowMapIndex = uLights[lightIndex].shadowMapIndex;

    vec4 positionLightClipH = uShadow.spotLights[spotLightShadowMapIndex].worldToClip * vec4(position, 1.0);
    vec3 positionLightClip = positionLightClipH.xyz / positionLightClipH.w;
    vec2 lightUv = positionLightClip.xy * 0.5 + 0.5;
    float lightDepth = positionLightClip.z * 0.5 + 0.5;
    
    float lightNearClip = uShadow.spotLights[spotLightShadowMapIndex].nearClip;
    float lightFarClip = uShadow.spotLights[spotLightShadowMapIndex].farClip;
    float lightSize = uShadow.spotLights[spotLightShadowMapIndex].size;
    float lightLinearDepth = linearizeDepth(lightDepth, lightNearClip, lightFarClip);
    
    vec2 texelSize = 1.0 / textureSize(uShading_SpotLightShadowMaps[spotLightShadowMapIndex], 0);
    // float avgScale = 1.0;
    float avgBlockerLightLinearDepth = 0.0;
    avgBlockerLightLinearDepth += linearizeDepth(
        texture(uShading_SpotLightShadowMaps[spotLightShadowMapIndex], lightUv + vec2(-1.0, -1.0) * texelSize).r,
        lightNearClip,
        lightFarClip);
    avgBlockerLightLinearDepth += linearizeDepth(
        texture(uShading_SpotLightShadowMaps[spotLightShadowMapIndex], lightUv + vec2(-1.0, 1.0) * texelSize).r,
        lightNearClip,
        lightFarClip);
    avgBlockerLightLinearDepth += linearizeDepth(
        texture(uShading_SpotLightShadowMaps[spotLightShadowMapIndex], lightUv + vec2(1.0, -1.0) * texelSize).r,
        lightNearClip,
        lightFarClip);
    avgBlockerLightLinearDepth += linearizeDepth(
        texture(uShading_SpotLightShadowMaps[spotLightShadowMapIndex], lightUv + vec2(1.0, 1.0) * texelSize).r,
        lightNearClip,
        lightFarClip);
    avgBlockerLightLinearDepth *= 0.25;
    
    float penumbraSize = (lightLinearDepth - avgBlockerLightLinearDepth) * lightSize / avgBlockerLightLinearDepth;
    float bias = max(0.0005 * (1.0 - dot(normal, normalize(lightPosition - position))), 0.00005);
    float shadow = 0.0;
    int kernelHalfSize = max(1, min(int(penumbraSize), 10));
    float kernelScale = max(1.0, sqrt(penumbraSize / kernelHalfSize));
    for (int x = -kernelHalfSize; x <= kernelHalfSize; ++x)
    {
        for (int y = -kernelHalfSize; y <= kernelHalfSize; ++y)
        {
            float pcfDepth =
                texture(uShading_SpotLightShadowMaps[spotLightShadowMapIndex], lightUv + vec2(x, y) * texelSize * kernelScale).r;
            shadow += lightDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    int kernelSize = 2 * kernelHalfSize + 1;
    shadow /= kernelSize * kernelSize;
    // shadow = smoothstep(0.0, 1.0, shadow);
    return shadow;
}

vec3 uEvaluateLight(int lightIndex, vec3 position, vec3 normal, vec3 albedo, float metallic, float roughness, float reflectance)
{
    return evaluateLight(uLights[lightIndex], uView.viewToWorld[3].xyz, position, normal, albedo, metallic, roughness, reflectance);
}

vec3 uEvaluateLights(
    vec4 coord,
    vec3 position,
    vec3 normal,
    vec3 albedo,
    float metallic,
    float roughness,
    float reflectance)
{
    vec3 color = vec3(0.0);
    
    int lightClusterIndex = uComputeLightClusterIndex();
    int lightClusterSize = uLightClusterSizes[lightClusterIndex];
    for (int i = 0; i < lightClusterSize; ++i)
    {
        int lightIndex = uLightClusterIndices[lightClusterIndex * uLighting.lightClusterCapacity + i];
        float shadow = uEvaluateShadow(lightIndex, position, normal);
        
        color += (1.0 - shadow) * uEvaluateLight(lightIndex, position, normal, albedo, metallic, roughness, reflectance);
    }
    
    float fragViewDepth = dot(position - uView.debugViewToWorld[3].xyz, -normalize(uView.debugViewToWorld[2].xyz));
    int sunCsmIndex = uShadow.sunCascadingShadowMapCount - 1;
    for (int i = 0; i < uShadow.sunCascadingShadowMapCount; ++i)
    {
        if (fragViewDepth < uShadow.sun[i].maxViewDepth)
        {
            sunCsmIndex = i;
            break;
        }
    }

    vec4 sunShadowClipPos = uShadow.sun[sunCsmIndex].worldToClip * vec4(position, 1.0);
    vec3 sunShadowNdc = sunShadowClipPos.xyz / sunShadowClipPos.w;
    vec2 sunShadowUv = sunShadowNdc.xy * 0.5 + 0.5;

    float sunShadowDepth = sunShadowNdc.z * 0.5 + 0.5;
    float sunSampledDepth = texture(uShading_SunShadowMap, vec3(sunShadowUv, sunCsmIndex)).r;
    float sunShadow = sunShadowDepth > sunSampledDepth + 0.0005 ? 1.0 : 0.0;
    color += albedo * (1.0 - sunShadow) * uLighting.sunColor * uLighting.sunIntensity;

    color += albedo * uLighting.ambientColor * texture(uShading_AmbientOcclusion, coord.xy * uView.invResolution).r;
    
    return color;
}

#endif // #ifndef VOB_AOEGL_SHADING_UTILS_GLSL