#version 450 core
#extension GL_NV_gpu_shader5 : enable

#include "defines.glsl"

layout(local_size_x = LIGHT_CLUSTERING_WORK_GROUP_SIZE) in;

#include "bindings.glsl"
#include "light_utils.glsl"

void main()
{
    ivec2 clusterXYCount = ivec2(ceil(vec2(uLighting.lightClusterResolution) / float(uLighting.lightClusterTileSize)));
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
        uLighting.lightClusterResolution,
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
                uLightClusterIndices[clusterIndex * uLighting.lightClusterCapacity + lightClusterSize] = i;
                lightClusterSize += 1;
            }
        }
    }
    uLightClusterSizes[clusterIndex] = lightClusterSize;
}
