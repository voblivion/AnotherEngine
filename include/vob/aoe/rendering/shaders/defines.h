#ifndef VOB_AOEGL_CORE_DEFINES_GLSL
#define VOB_AOEGL_CORE_DEFINES_GLSL

#define SUN_CASCADING_SHADOW_MAPS_CAPACITY 4
#define SPOT_LIGHT_SHADOW_MAPS_CAPACITY 6
#define RIG_BONES_CAPACITY 100
#define MATERIAL_TEXTURES_CAPACITY 16
// Note: max int16
#define MAX_LIGHTS_CAPACITY 32767
// Note: max int8
#define MAX_LIGHT_CLUSTER_CAPACITY 127

#define BINDING_UBO_GLOBAL 0
#define BINDING_UBO_VIEW 1
#define BINDING_UBO_LIGHTING 2
#define BINDING_UBO_SHADOW 3
#define BINDING_UBO_MODEL 4
#define BINDING_UBO_RIG 5
#define BINDING_UBO_CUSTOM 6
#define BINDING_UBO_MATERIAL BINDING_UBO_CUSTOM
#define BINDING_UBO_SSAO BINDING_UBO_CUSTOM
#define BINDING_UBO_SSR BINDING_UBO_CUSTOM
#define BINDING_UBO_POST_PROCESS BINDING_UBO_CUSTOM
#define BINDING_UBO_DEBUG BINDING_UBO_CUSTOM

#define BINDING_SSBO_LIGHTS 0
#define BINDING_SSBO_LIGHT_CLUSTER_SIZES 1
#define BINDING_SSBO_LIGHT_CLUSTER_INDICES 2

#define BINDING_TEXTURE_SSAO_OPAQUE_DEPTH 0
#define BINDING_TEXTURE_SSAO_OPAQUE_GEOMETRIC_NORMAL 1

#define BINDING_TEXTURE_MATERIAL_AMBIENT_OCCLUSION 0
#define BINDING_TEXTURE_MATERIAL_SUN_SHADOW_MAP 1
#define BINDING_TEXTURE_MATERIAL_SPOT_LIGHT_SHADOW_MAPS_BEGIN 2
#define BINDING_TEXTURE_MATERIAL_CUSTOMS_BEGIN BINDING_TEXTURE_MATERIAL_SPOT_LIGHT_SHADOW_MAPS_BEGIN + SPOT_LIGHT_SHADOW_MAPS_CAPACITY

#define BINDING_TEXTURE_SSR_DIRECT_OPAQUE_COLOR 0
#define BINDING_TEXTURE_SSR_OPAQUE_SURFACE 1
#define BINDING_TEXTURE_SSR_OPAQUE_NORMAL 2
#define BINDING_TEXTURE_SSR_OPAQUE_DEPTH 3

#define BINDING_TEXTURE_OPAQUE_COMPOSITION_DIRECT_OPAQUE_COLOR 0
#define BINDING_TEXTURE_OPAQUE_COMPOSITION_OPAQUE_SURFACE 1
#define BINDING_TEXTURE_OPAQUE_COMPOSITION_SSR_COLOR 2
#define BINDING_TEXTURE_OPAQUE_COMPOSITION_ENVIRONMENT_CUBE_MAP 3

#define BINDING_TEXTURE_POST_PROCESS_COLOR 0

#define BINDING_TEXTURE_DEBUG 0
#define BINDING_TEXTURE_ARRAY_DEBUG 1

#define DEBUG_TYPE_COLOR_TEXTURE 0
#define DEBUG_TYPE_SHADES_TEXTURE 1
#define DEBUG_TYPE_DEPTH_TEXTURE 2
#define DEBUG_TYPE_DEPTH_TEXTURE_ARRAY 3
#define DEBUG_TYPE_DIRECTION_TEXTURE 4
#define DEBUG_TYPE_LIGHT_CLUSTERS 5

#ifdef __cplusplus
#include <glm/glm.hpp>
#include <array>
#define vec2 glm::vec2
#define vec3 glm::vec3
#define vec4 glm::vec4
#define ivec2 glm::ivec2
#define ivec3 glm::ivec3
#define ivec4 glm::ivec4
#define mat2 glm::mat2
#define mat3 glm::mat3
#define mat4 glm::mat4
#define ubo_vec2 alignas(8) vec2
#define ubo_vec3 alignas(16) vec3
#define ubo_vec4 alignas(16) vec4
#define ubo_ivec2 alignas(8) ivec2
#define ubo_ivec3 alignas(16) ivec3
#define ubo_ivec4 alignas(16) ivec4
#define ubo_mat2 alignas(16) mat2
#define ubo_mat3 alignas(16) mat3
#define ubo_mat4 alignas(16) mat4
#define ALIGN_16 alignas(16)
#define ARRAY(Type, Size, Name) std::array<Type, Size> Name

namespace vob::aoegl
{
#else
#define ubo_vec2 vec2
#define ubo_vec3 vec3
#define ubo_vec4 vec4
#define ubo_ivec2 ivec2
#define ubo_ivec3 ivec3
#define ubo_ivec4 ivec4
#define ubo_mat2 mat2
#define ubo_mat3 mat3
#define ubo_mat4 mat4
#define ALIGN_16
#define ARRAY(Type, Size, Name) Type Name[Size]
#endif

struct ALIGN_16 UniformGlobalParams
{
    float worldTime;
};

struct ALIGN_16 UniformViewParams
{
    ubo_mat4 worldToView;
    ubo_mat4 viewToClip;
    ubo_mat4 worldToClip;
    ubo_mat4 clipToView;
    ubo_mat4 viewToWorld;
    ubo_ivec2 resolution;
    ubo_vec2 invResolution;
    float nearClip;
    float farClip;
    // Not really needed for glsl, but handy + there is room
    float fov;
    float aspectRatio;
};

struct ALIGN_16 UniformLightingParams
{
    ubo_vec3 ambientColor;
    int lightCount;
    ubo_ivec2 lightClusterTileSize;
    int lightClusterZCount;
    int lightClusterCapacity;
    ubo_vec3 sunColor;
    float sunIntensity;
    ubo_vec3 sunDir;
};

struct ALIGN_16 GpuSunCascadingShadow
{
    ubo_mat4 worldToClip;
    float maxViewDepth;
    float nearClip;
    float farClip;
};

struct ALIGN_16 GpuSpotLightShadow
{
    ubo_mat4 worldToClip;
    float nearClip;
    float farClip;
    float size;
    // Not really needed for glsl, but handy + there is room
    float fov;
    // Not really needed for glsls and increases needed bandwith
    ubo_mat4 viewToWorld;
};

struct ALIGN_16 UniformShadowParams
{
    ARRAY(GpuSunCascadingShadow, SUN_CASCADING_SHADOW_MAPS_CAPACITY, sun);
    ARRAY(GpuSpotLightShadow, SPOT_LIGHT_SHADOW_MAPS_CAPACITY, spotLights);
    ubo_mat4 sunReferenceViewToWorld;
    int sunCascadingShadowMapCount;
};

struct ALIGN_16 UniformModelParams
{
    ubo_mat4 modelToWorld;
};

#ifdef __cplusplus
inline bool operator==(UniformModelParams const& a_lhs, UniformModelParams const& a_rhs)
{
    return a_lhs.modelToWorld == a_rhs.modelToWorld;
}
#endif

struct ALIGN_16 UniformRigParams
{
    ARRAY(mat4, RIG_BONES_CAPACITY, bones);
};

struct ALIGN_16 GpuLight
{
    vec3 position;
    float radius;
    vec3 color;
    float intensity;
    vec3 direction;
    int type;
    float outerAngleCos;
    float innerAngleCos;
    int shadowMapIndex;
};

struct ALIGN_16 UniformSsrParams
{
    int log2Step;
    int log2SubStep;
    float thicknessRatio;
    float maxRangeRatio;
    float initialBiasRatio;
    float maxThickness;
};

struct ALIGN_16 UniformSsaoParams
{
    int sampleCount;
    float radius;
    float attenuationBias;
    float attenuationScale;
    float threshold;
};

struct ALIGN_16 UniformDebugParams
{
    int8_t type;
    int8_t index;
};

#ifdef __cplusplus
    static constexpr int32_t k_sunCascadingShadowMapsCapacity = SUN_CASCADING_SHADOW_MAPS_CAPACITY;
	static constexpr int32_t k_spotLightShadowMapsCapacity = SPOT_LIGHT_SHADOW_MAPS_CAPACITY;
	static constexpr int32_t k_rigBonesCapacity = RIG_BONES_CAPACITY;
	static constexpr int32_t k_materialTexturesCapacity = MATERIAL_TEXTURES_CAPACITY;
	static constexpr int32_t k_maxLightsCapacity = MAX_LIGHTS_CAPACITY;
	static constexpr int32_t k_maxLightClusterCapacity = MAX_LIGHT_CLUSTER_CAPACITY;

	static constexpr uint32_t k_bindingUboGlobal = BINDING_UBO_GLOBAL;
	static constexpr uint32_t k_bindingUboView = BINDING_UBO_VIEW;
	static constexpr uint32_t k_bindingUboLighting = BINDING_UBO_LIGHTING;
	static constexpr uint32_t k_bindingUboShadow = BINDING_UBO_SHADOW;
    static constexpr uint32_t k_bindingUboModel = BINDING_UBO_MODEL;
    static constexpr uint32_t k_bindingUboRig = BINDING_UBO_RIG;
	static constexpr uint32_t k_bindingUboMaterial = BINDING_UBO_MATERIAL;
	static constexpr uint32_t k_bindingUboSsao = BINDING_UBO_SSAO;
	static constexpr uint32_t k_bindingUboSsr = BINDING_UBO_SSR;
	static constexpr uint32_t k_bindingUboPostProcess = BINDING_UBO_POST_PROCESS;
	static constexpr uint32_t k_bindingUboDebug = BINDING_UBO_DEBUG;

	static constexpr uint32_t k_bindingSsboLights = BINDING_SSBO_LIGHTS;
	static constexpr uint32_t k_bindingSsboLightClusterSizes = BINDING_SSBO_LIGHT_CLUSTER_SIZES;
	static constexpr uint32_t k_bindingSsboLightClusterIndices = BINDING_SSBO_LIGHT_CLUSTER_INDICES;

    static constexpr uint32_t k_bindingTextureSsaoOpaqueDepth = BINDING_TEXTURE_SSAO_OPAQUE_DEPTH;
    static constexpr uint32_t k_bindingTextureSsaoOpaqueGeometricNormal = BINDING_TEXTURE_SSAO_OPAQUE_GEOMETRIC_NORMAL;

	static constexpr uint32_t k_bindingTextureMaterialAmbientOcclusion = BINDING_TEXTURE_MATERIAL_AMBIENT_OCCLUSION;
	static constexpr uint32_t k_bindingTextureMaterialSunShadowMap = BINDING_TEXTURE_MATERIAL_SUN_SHADOW_MAP;
	static constexpr uint32_t k_bindingTextureMaterialSpotLightShadowMapsBegin = BINDING_TEXTURE_MATERIAL_SPOT_LIGHT_SHADOW_MAPS_BEGIN;
	static constexpr uint32_t k_bindingTextureMaterialCustomsBegin = BINDING_TEXTURE_MATERIAL_CUSTOMS_BEGIN;

	static constexpr uint32_t k_bindingTextureSsrDirectOpaqueColor = BINDING_TEXTURE_SSR_DIRECT_OPAQUE_COLOR;
	static constexpr uint32_t k_bindingTextureSsrOpaqueSurface = BINDING_TEXTURE_SSR_OPAQUE_SURFACE;
	static constexpr uint32_t k_bindingTextureSsrOpaqueNormal = BINDING_TEXTURE_SSR_OPAQUE_NORMAL;
	static constexpr uint32_t k_bindingTextureSsrOpaqueDepth = BINDING_TEXTURE_SSR_OPAQUE_DEPTH;

	static constexpr uint32_t k_bindingTextureOpaqueCompositionDirectOpaqueColor = BINDING_TEXTURE_OPAQUE_COMPOSITION_DIRECT_OPAQUE_COLOR;
	static constexpr uint32_t k_bindingTextureOpaqueCompositionOpaqueSurface = BINDING_TEXTURE_OPAQUE_COMPOSITION_OPAQUE_SURFACE;
	static constexpr uint32_t k_bindingTextureOpaqueCompositionSsrColor = BINDING_TEXTURE_OPAQUE_COMPOSITION_SSR_COLOR;
	static constexpr uint32_t k_bindingTextureOpaqueCompositionEnvironmentCubeMap = BINDING_TEXTURE_OPAQUE_COMPOSITION_ENVIRONMENT_CUBE_MAP;

	static constexpr uint32_t k_bindingTexturePostProcessColor = BINDING_TEXTURE_POST_PROCESS_COLOR;
    
	static constexpr uint32_t k_bindingTextureDebug = BINDING_TEXTURE_DEBUG;
	static constexpr uint32_t k_bindingTextureArrayDebug = BINDING_TEXTURE_ARRAY_DEBUG;
    
    enum class DebugType : uint8_t
    {
        ColorTexture = DEBUG_TYPE_COLOR_TEXTURE,
        ShadesTexture = DEBUG_TYPE_SHADES_TEXTURE,
        DepthTexture = DEBUG_TYPE_DEPTH_TEXTURE,
        DepthTextureArray = DEBUG_TYPE_DEPTH_TEXTURE_ARRAY,
        DirectionTexture = DEBUG_TYPE_DIRECTION_TEXTURE,
        LightClusters = DEBUG_TYPE_LIGHT_CLUSTERS,
    };
}

#undef SPOT_LIGHT_SHADOW_MAPS_CAPACITY
#undef RIG_BONES_CAPACITY
#undef MATERIAL_TEXTURES_CAPACITY
#undef MAX_LIGHTS_CAPACITY
#undef MAX_LIGHT_CLUSTER_CAPACITY

#undef BINDING_UBO_GLOBAL
#undef BINDING_UBO_VIEW
#undef BINDING_UBO_LIGHTING
#undef BINDING_UBO_SHADOW
#undef BINDING_UBO_CUSTOM
#undef BINDING_UBO_MATERIAL
#undef BINDING_UBO_SSR
#undef BINDING_UBO_POST_PROCESS
#undef BINDING_UBO_MODEL
#undef BINDING_UBO_RIG

#undef BINDING_SSBO_LIGHTS
#undef BINDING_SSBO_LIGHT_CLUSTER_SIZES
#undef BINDING_SSBO_LIGHT_CLUSTER_INDICES

#undef BINDING_TEXTURE_SHADING_SUN_SHADOW_MAP
#undef BINDING_TEXTURE_SHADING_SPOT_LIGHT_SHADOW_MAP_BEGIN
#undef BINDING_TEXTURE_SHADING_MATERIAL_BEGIN

#undef BINDING_TEXTURE_SSR_DIRECT_OPAQUE_COLOR
#undef BINDING_TEXTURE_SSR_OPAQUE_SURFACE
#undef BINDING_TEXTURE_SSR_OPAQUE_NORMAL
#undef BINDING_TEXTURE_SSR_OPAQUE_DEPTH

#undef BINDING_TEXTURE_OPAQUE_COMPOSITION_LIT_OPAQUE_COLOR
#undef BINDING_TEXTURE_OPAQUE_COMPOSITION_OPAQUE_SURFACE
#undef BINDING_TEXTURE_OPAQUE_COMPOSITION_SSR_COLOR
#undef BINDING_TEXTURE_OPAQUE_COMPOSITION_ENVIRONMENT_CUBE_MAP

#undef BINDING_TEXTURE_POST_PROCESS_COLOR

#undef DEBUG_TYPE_COLOR
#undef DEBUG_TYPE_DEPTH
#undef DEBUG_TYPE_DIRECTION

#undef vec2
#undef vec3
#undef vec4
#undef ivec2
#undef ivec3
#undef ivec4
#undef mat2
#undef mat3
#undef mat4
#endif // #ifdef __cplusplus

#undef ubo_vec2
#undef ubo_vec3
#undef ubo_vec4
#undef ubo_ivec2
#undef ubo_ivec3
#undef ubo_ivec4
#undef ubo_mat2
#undef ubo_mat3
#undef ubo_mat4
#undef ALIGN_16
#undef ARRAY

#endif // #ifndef VOB_AOEGL_CORE_DEFINES_GLSL