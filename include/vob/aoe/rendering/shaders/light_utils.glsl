#ifndef VOB_AOEGL_CORE_LIGHT_UTILS_GLSL
#define VOB_AOEGL_CORE_LIGHT_UTILS_GLSL

#include "core/math_utils.glsl"

struct LightClusterBounds
{
    vec3 minView;
    vec3 maxView;
};

LightClusterBounds computeLightClusterBounds(
    int clusterIdx,
    ivec2 tileSize,
    int depthSliceCount,
    ivec2 resolution,
    mat4 clipToView,
    float nearClip,
    float farClip)
{
    int colCount = (resolution.x + tileSize.x - 1) / tileSize.x;
    int rowCount = (resolution.y + tileSize.y - 1) / tileSize.y;
    int depthSliceTileCount = colCount * rowCount;
    int tileZ = clusterIdx / depthSliceTileCount;
    int tileZRemainder = clusterIdx % depthSliceTileCount;
    int tileY = tileZRemainder / colCount;
    int tileX = tileZRemainder % colCount;
    
    vec4 minClip = vec4(vec2(tileX, tileY) * vec2(tileSize) / vec2(resolution) * 2.0 - 1.0, -1.0, 1.0);
    vec4 maxClip = vec4(vec2(tileX + 1, tileY + 1) * vec2(tileSize) / vec2(resolution) * 2.0 - 1.0, -1.0, 1.0);
    vec4 minViewH = clipToView * minClip;
    vec4 maxViewH = clipToView * maxClip;
    vec3 minView = minViewH.xyz / minViewH.w;
    vec3 maxView = maxViewH.xyz / maxViewH.w;
    
    float clipRatio = farClip / nearClip;
    float clusterNear = nearClip * pow(clipRatio, float(tileZ) / float(depthSliceCount));
    float clusterFar = nearClip * pow(clipRatio, float(tileZ + 1) / float(depthSliceCount));
    
    vec3 minNear = intersectViewDepthPlane(minView, clusterNear);
    vec3 minFar = intersectViewDepthPlane(minView, clusterFar);
    vec3 maxNear = intersectViewDepthPlane(maxView, clusterNear);
    vec3 maxFar = intersectViewDepthPlane(maxView, clusterFar);
    
    LightClusterBounds bounds;
    bounds.minView = min(min(minNear, minFar), min(maxNear, maxFar));
    bounds.maxView = max(max(minNear, minFar), max(maxNear, maxFar));
    
    return bounds;
}

bool testIntersectLightClusterBounds(LightClusterBounds bounds, vec3 position, float radius, mat4 worldToView)
{
    vec3 positionView = (worldToView * vec4(position, 1.0)).xyz;
    vec3 closest = clamp(positionView, bounds.minView, bounds.maxView);
    vec3 diff = positionView - closest;
    return dot(diff, diff) <= radius * radius;
}

int computeLightClusterIndex(
    vec2 fragCoord,
    float depth,
    ivec2 tileSize,
    int depthSliceCount,
    ivec2 resolution,
    float nearClip,
    float farClip)
{
    int colCount = (resolution.x + tileSize.x - 1) / tileSize.x;
    int rowCount = (resolution.y + tileSize.y - 1) / tileSize.y;
    int tileX = clamp(int(fragCoord.x) / tileSize.x, 0, colCount - 1);
    int tileY = clamp(int(fragCoord.y) / tileSize.y, 0, rowCount - 1);
    
    float zClip = depth * 2.0 - 1.0;
    float zView = (2.0 * nearClip * farClip) / (farClip + nearClip - zClip * (farClip - nearClip));
    
    float zRatio = log(zView / nearClip) / log(farClip / nearClip);
    int tileZ = clamp(int(zRatio * depthSliceCount), 0, depthSliceCount - 1);
    return tileX + tileY * colCount + tileZ * colCount * rowCount;
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float b = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265358979 * b * b);
}

float geometrySchlickGGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(float NdotV, float NdotL, float roughness)
{
    float r = (roughness + 1.0);
    // Note: higher 8.0 for less darkening at grazing angles; Epics maybe uses 8.0?
    float k = (r * r) / 2.0;
    float ggx2 = geometrySchlickGGX(NdotV, k);
    float ggx1 = geometrySchlickGGX(NdotL, k);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float evaluateLightAttenuation(GpuLight light, vec3 toLightDir, float toLightDist)
{
    float isSpot = float(light.type);
    float isPoint = 1.0 - isSpot;
    
    float relativeDist = toLightDist / light.radius;
    float relativeDistSq = relativeDist * relativeDist;
    float distanceAttenuation = clamp(1.0 - relativeDistSq, 0.0, 1.0);
    
    float cosTheta = dot(light.direction, -toLightDir);
    float spotAngleAttenuation = smoothstep(light.outerAngleCos, light.innerAngleCos, cosTheta);
    float angleAttenuation = mix(1.0, spotAngleAttenuation, isSpot);
    
    return distanceAttenuation * angleAttenuation;
}

vec3 evaluateLight(
    GpuLight light,
    vec3 viewPosition,
    vec3 position,
    vec3 normal,
    vec3 albedo,
    float metallic,
    float roughness,
    float reflectance)
{
    vec3 toLight = light.position - position;
    float toLightDist = length(toLight);
    
    vec3 L = toLight / toLightDist;
    vec3 N = normal;
    vec3 V = normalize(viewPosition - position);
    vec3 H = normalize(V + L);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    
    float attenuation = evaluateLightAttenuation(light, L, toLightDist);
    vec3 radiance = light.color * light.intensity * attenuation;
    
    vec3 F0 = mix(vec3(0.16 * reflectance * reflectance), albedo, metallic);
    
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(NdotV, NdotL, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;
    vec3 specular = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.0001);
    
    return vec3((kD * albedo / 3.14159265358979 + specular) * radiance * NdotL);
}

#endif // VOB_AOEGL_CORE_LIGHT_UTILS_GLSL