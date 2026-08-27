#ifndef VOB_AOEGL_CORE_BINDINGS_SSR_GLSL
#define VOB_AOEGL_CORE_BINDINGS_SSR_GLSL

#include "core/bindings.glsl"

layout(binding = BINDING_TEXTURE_SSR_DIRECT_OPAQUE_COLOR) uniform sampler2D uSsr_DirectOpaqueColor;
layout(binding = BINDING_TEXTURE_SSR_OPAQUE_SURFACE) uniform sampler2D uSsr_OpaqueSurface;
layout(binding = BINDING_TEXTURE_SSR_OPAQUE_NORMAL) uniform sampler2D uSsr_OpaqueNormal;
layout(binding = BINDING_TEXTURE_SSR_OPAQUE_DEPTH) uniform sampler2D uSsr_OpaqueDepth;
layout(binding = BINDING_TEXTURE_SSR_HIZ_DEPTH) uniform sampler2D uSsr_HiZDepth;
layout(binding = BINDING_TEXTURE_SSR_AMBIENT_OCCLUSION) uniform sampler2D uSsr_AmbientOcclusion;

#endif // #ifndef VOB_AOEGL_CORE_BINDINGS_SSR_GLSL
