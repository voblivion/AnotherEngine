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

layout(binding = BINDING_TEXTURE_SSAO_OPAQUE_GEOMETRIC_NORMAL) uniform sampler2D uSsao_OpaqueGeometricNormal;
layout(binding = BINDING_TEXTURE_SSAO_OPAQUE_DEPTH) uniform sampler2D uSsao_OpaqueDepth;

layout(binding = BINDING_TEXTURE_SHADING_AMBIENT_OCCLUSION) uniform sampler2D uShading_AmbientOcclusion;
layout(binding = BINDING_TEXTURE_SHADING_SUN_SHADOW_MAP) uniform sampler2DArray uShading_SunShadowMap;
layout(binding = BINDING_TEXTURE_SHADING_SPOT_LIGHT_SHADOW_MAPS_BEGIN) uniform sampler2D uShading_SpotLightShadowMaps[SPOT_LIGHT_SHADOW_MAPS_CAPACITY];

layout(binding = BINDING_TEXTURE_SSR_DIRECT_OPAQUE_COLOR) uniform sampler2D uSsr_DirectOpaqueColor;
layout(binding = BINDING_TEXTURE_SSR_OPAQUE_SURFACE) uniform sampler2D uSsr_OpaqueSurface;
layout(binding = BINDING_TEXTURE_SSR_OPAQUE_NORMAL) uniform sampler2D uSsr_OpaqueNormal;
layout(binding = BINDING_TEXTURE_SSR_OPAQUE_DEPTH) uniform sampler2D uSsr_OpaqueDepth;

layout(binding = BINDING_TEXTURE_OPAQUE_COMPOSITION_DIRECT_OPAQUE_COLOR) uniform sampler2D uOpaqueComposition_DirectOpaqueColor;
layout(binding = BINDING_TEXTURE_OPAQUE_COMPOSITION_OPAQUE_SURFACE) uniform sampler2D uOpaqueComposition_OpaqueSurface;
layout(binding = BINDING_TEXTURE_OPAQUE_COMPOSITION_SSR_COLOR) uniform sampler2D uOpaqueComposition_SsrColor;

layout(binding = BINDING_TEXTURE_BLOOM_SOURCE) uniform sampler2D uBloom_Source;

layout(binding = BINDING_TEXTURE_BLOOM_COMBINE_SCENE) uniform sampler2D uBloomCombine_Scene;
layout(binding = BINDING_TEXTURE_BLOOM_COMBINE_BLOOM) uniform sampler2D uBloomCombine_Bloom;

layout(binding = BINDING_TEXTURE_TONEMAP_SOURCE) uniform sampler2D uTonemap_Source;

layout(binding = BINDING_TEXTURE_ANTI_ALIASING_SOURCE) uniform sampler2D uAntiAliasing_Source;

layout(binding = BINDING_TEXTURE_PRESENT_SOURCE) uniform sampler2D uPresent_Source;

layout(binding = BINDING_TEXTURE_DEBUG_SOURCE) uniform sampler2D uDebug_Source;
#endif // #ifndef VOB_AOEGL_CORE_BINDINGS_GLSL