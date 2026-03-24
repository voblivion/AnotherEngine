#version 450 core
#extension GL_NV_gpu_shader5 : enable

layout(local_size_x = WORK_GROUP_SIZE) in;

#include "core/bindings.glsl"
#include "core/light_utils.glsl"

void main()
{
    ivec2 clusterXYCount = ivec2(ceil(vec2(uView.resolution) / float(uLighting.lightClusterTileSize)));
    int clusterCount = clusterXYCount.x * clusterXYCount.y * uLighting.lightClusterZCount;

    int clusterIndex = int(gl_GlobalInvocationID.x);
    if (clusterIndex >= clusterCount)
    {
        return;
    }
    
    LightClusterBounds bounds = computeLightClusterBounds(
        clusterIndex,
        uLighting.lightClusterTileSize,
        uLighting.lightClusterZCount,
        uView.resolution,
        uView.clipToView,
        uView.nearClip,
        uView.farClip);
    
    int lightClusterSize = 0;
    for (int i = 0; i < uLighting.lightCount; ++i)
    {
        if (testIntersectLightClusterBounds(bounds, uLights[i].position, uLights[i].radius, uView.worldToView))
        {
            if (lightClusterSize < uLighting.lightClusterCapacity)
            {
                uLightClusterIndices[clusterIndex * uLighting.lightClusterCapacity + lightClusterSize] = int16_t(i);
                lightClusterSize += 1;
            }
        }
    }
    uLightClusterSizes[clusterIndex] = int8_t(lightClusterSize);
}
