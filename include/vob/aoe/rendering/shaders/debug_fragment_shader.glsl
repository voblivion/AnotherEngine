#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/light_utils.glsl"
#include "core/math_utils.glsl"

in vec2 vUv;
out vec4 oColor;


vec3 heat7Colors(float ratio)
{
    vec3 colors[7] = vec3[7](
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, 0.0, 1.0),
        vec3(0.0, 1.0, 1.0),
        vec3(0.0, 1.0, 0.0),
        vec3(1.0, 1.0, 0.0),
        vec3(1.0, 0.0, 0.0),
        vec3(1.0, 1.0, 1.0)
    );
    
    if (ratio >= 1)
    {
        return vec3(1.0);
    }
    float scaled = clamp(ratio, 0.0, 1.0) * 5.0;
    int index = int(floor(scaled));
    float t = scaled - float(index);
    return mix(colors[index], colors[index+1], t);
}

void main()
{
    vec3 c;
    if (uDebug.type == DEBUG_TYPE_SHADES_TEXTURE)
    {
        c = vec3(texture(uDebug_Texture, vUv).r);
    }
    else if (uDebug.type == DEBUG_TYPE_DEPTH_TEXTURE)
    {
        float linearDepth = linearizeDepth(texture(uDebug_Texture, vUv).r, uView.nearClip, uView.farClip);
        // c = vec3(linearizeDepth(c.r, uView.nearClip, uView.farClip) / uView.farClip);
        // c = heat7Colors(log(linearizeDepth(c.r, uView.nearClip, uView.farClip) + 1.0) / log(uView.farClip + 1.0));
        // c = heat7Colors(linearizeDepth(c.r, uView.nearClip, uView.farClip) / uView.farClip);
        float normalizedLinearDepth = (linearDepth - uView.nearClip) / (uView.farClip - uView.nearClip);
        float k = 10.0;
        c = heat7Colors(log(normalizedLinearDepth * k + 1.0) / log(k + 1.0));
    }
    else if (uDebug.type == DEBUG_TYPE_DEPTH_TEXTURE_ARRAY)
    {
        float d = texture(uDebug_TextureArray, vec3(vUv, uDebug.index)).r;
        float k = 10.0;
        c = heat7Colors(log(d * k + 1.0) / log(k + 1.0));
    }
    else if (uDebug.type == DEBUG_TYPE_DIRECTION_TEXTURE)
    {
        c = texture(uDebug_Texture, vUv).rgb * 0.5 + 0.5;
    }
    else if (uDebug.type == DEBUG_TYPE_LIGHT_CLUSTERS)
    {
        if (texture(uDebug_Texture, vUv).r < 1.0)
        {
            int lightClusterIndex = computeLightClusterIndex(
                gl_FragCoord.xy,
                texture(uDebug_Texture, vUv).r,
                uLighting.lightClusterTileSize,
                uLighting.lightClusterZCount,
                uView.resolution,
                uView.nearClip,
                uView.farClip);
            int lightClusterSize = uLightClusterSizes[lightClusterIndex];
            c = heat7Colors(float(lightClusterSize) / uLighting.lightClusterCapacity);
        }
        else
        {
            c = heat7Colors(0.0);
        }
    }
    
    oColor = vec4(c, 1.0);
}
