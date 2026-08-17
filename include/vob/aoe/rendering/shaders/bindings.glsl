#ifndef VOB_AOEGL_CORE_BINDINGS_GLSL
#define VOB_AOEGL_CORE_BINDINGS_GLSL

#include "core/defines.glsl"

layout(std140, binding = BINDING_UBO_GLOBAL) uniform GlobalParams
{
    UniformGlobalParams uGlobal;
};

layout(std140, binding = BINDING_UBO_VIEW) uniform ViewParams
{
    UniformViewParams uView;
};

layout(std140, binding = BINDING_UBO_TARGET) uniform TargetParams
{
    UniformTargetParams uTarget;
};

layout(std140, binding = BINDING_UBO_LIGHTING) uniform LightingParams
{
    UniformLightingParams uLighting;
};

layout(std140, binding = BINDING_UBO_SHADOW) uniform ShadowParams
{
    UniformShadowParams uShadow;
};

layout(std140, binding = BINDING_UBO_MODEL) uniform ModelParams
{
    UniformModelParams uModel;
};

layout(std140, binding = BINDING_UBO_RIG) uniform RigParams
{
    UniformRigParams uRig;
};

layout(std140, binding = BINDING_UBO_SSR) uniform SsrParams
{
    UniformSsrParams uSsr;
};

layout(std140, binding = BINDING_UBO_SSAO) uniform SsaoParams
{
    UniformSsaoParams uSsao;
};

layout(std140, binding = BINDING_UBO_BLOOM) uniform BloomParams
{
    UniformBloomParams uBloom;
};

layout(std140, binding = BINDING_UBO_TONEMAP) uniform TonemapParams
{
    UniformTonemapParams uTonemap;
};

layout(std140, binding = BINDING_UBO_DEBUG) uniform DebugParams
{
    UniformDebugParams uDebug;
};

layout(std430, binding = BINDING_SSBO_LIGHTS) readonly buffer ShaderStorageLights
{
    GpuLight uLights[];
};

layout(std430, binding = BINDING_SSBO_LIGHT_CLUSTER_SIZES) buffer ShaderStorageLightClusterSizes
{
    int8_t uLightClusterSizes[];
};

layout(std430, binding = BINDING_SSBO_LIGHT_CLUSTER_INDICES) buffer ShaderStorageLightClusterIndices
{
    int16_t uLightClusterIndices[];
};

layout(std430, binding = BINDING_SSBO_SKY_IRRADIANCE) buffer ShaderStorageSkyIrradiance
{
    vec4 uSkyIrradiance[SKY_IRRADIANCE_COEFFICIENT_COUNT];
};

// Samplers live in core/bindings_<pass>.glsl, one file per binding group: every declared sampler
// counts against GL_MAX_TEXTURE_IMAGE_UNITS even when unused, so a program must only declare its own.
#endif // #ifndef VOB_AOEGL_CORE_BINDINGS_GLSL