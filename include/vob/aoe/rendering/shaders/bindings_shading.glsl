#ifndef VOB_AOEGL_CORE_BINDINGS_SHADING_GLSL
#define VOB_AOEGL_CORE_BINDINGS_SHADING_GLSL

#include "core/bindings.glsl"

layout(binding = BINDING_TEXTURE_SHADING_AMBIENT_OCCLUSION) uniform sampler2D uShading_AmbientOcclusion;
layout(binding = BINDING_TEXTURE_SHADING_SSAO_LINEAR_DEPTH) uniform sampler2D uShading_SsaoLinearDepth;
layout(binding = BINDING_TEXTURE_SHADING_SUN_SHADOW_MAP) uniform sampler2DArray uShading_SunShadowMap;
layout(binding = BINDING_TEXTURE_SHADING_SPOT_LIGHT_SHADOW_MAPS_BEGIN) uniform sampler2D uShading_SpotLightShadowMaps[SPOT_LIGHT_SHADOW_MAPS_CAPACITY];

#endif // #ifndef VOB_AOEGL_CORE_BINDINGS_SHADING_GLSL
