#ifndef VOB_AOEGL_SHADING_UTILS_GLSL
#define VOB_AOEGL_SHADING_UTILS_GLSL

#include "core/bindings_shading.glsl"
#include "core/light_utils.glsl"
#include "core/shading_outputs.glsl"

int uComputeLightClusterIndex()
{
    return computeLightClusterIndex(
        gl_FragCoord.xy * uTarget.invResolution * vec2(uLighting.lightClusterResolution),
        gl_FragCoord.z,
        uLighting.lightClusterTileSize,
        uLighting.lightClusterZCount,
        uLighting.lightClusterResolution,
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

float uEvaluateSunShadow(vec3 position)
{
    float fragViewDepth = dot(position - uShadow.sunReferenceViewToWorld[3].xyz, -normalize(uShadow.sunReferenceViewToWorld[2].xyz));
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
    
    return sunShadowDepth > sunSampledDepth + 0.001 ? 1.0 : 0.0;
}

float uEvaluateAmbientOcclusion(vec4 coord)
{
    return texture(uShading_AmbientOcclusion, coord.xy * uTarget.invResolution).r;
}

// Geometric specular antialiasing: a normal that swings within one pixel cannot produce a mirror
// thin highlight, so widen roughness by how fast it varies on screen.
float uFilterSpecularRoughness(vec3 normal, float roughness)
{
    const float k_variance = 0.15;
    const float k_threshold = 0.25;

    vec3 dndx = dFdx(normal);
    vec3 dndy = dFdy(normal);
    float variance = k_variance * (dot(dndx, dndx) + dot(dndy, dndy));
    float kernelRoughness = min(2.0 * variance, k_threshold);

    float alpha = roughness * roughness;
    return sqrt(sqrt(clamp(alpha * alpha + kernelRoughness, 0.0, 1.0)));
}

vec3 uEvaluateLights(
    vec4 coord,
    vec3 position,
    vec3 normal,
    vec3 albedo,
    float metallic,
    float roughness,
    float reflectance,
    float occlusion)
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
    
    float sunShadow = uEvaluateSunShadow(position);
    float sunNdotL = clamp(dot(uLighting.sunDir, normal), 0.0, 1.0);
    vec3 viewDir = normalize(uView.viewToWorld[3].xyz - position);
    vec3 sunBrdf = evaluateBrdf(uLighting.sunDir, viewDir, normal, albedo, metallic, roughness, reflectance);
    color += sunBrdf * sunNdotL * (1.0 - sunShadow) * uLighting.sunColor * uLighting.sunIntensity;

    float ambientOcclusion = uEvaluateAmbientOcclusion(coord) * occlusion;
    vec3 irradiance = max(uEvaluateSkyIrradiance(normal), vec3(0.0));
    color += (1.0 - metallic) * albedo / 3.14159265358979 * irradiance * uLighting.ambientIntensity * ambientOcclusion;

    return color;
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
    return uEvaluateLights(coord, position, normal, albedo, metallic, roughness, reflectance, 1.0);
}

// The conventional way to fill OpaqueOutputs. A material free to shade itself differently simply
// does not call this.
OpaqueOutputs uShadeStandardOpaqueSurface(
    vec4 coord,
    vec3 position,
    vec3 normal,
    vec3 albedo,
    float metallic,
    float inRoughness,
    float reflectance,
    float occlusion)
{
    float roughness = uFilterSpecularRoughness(normal, inRoughness);

    OpaqueOutputs outputs;
    outputs.color = uEvaluateLights(coord, position, normal, albedo, metallic, roughness, reflectance, occlusion);
    outputs.normal = normal;
    outputs.surface = vec4(mix(vec3(0.16 * reflectance * reflectance), albedo, metallic), roughness);

    return outputs;
}

#endif // #ifndef VOB_AOEGL_SHADING_UTILS_GLSL