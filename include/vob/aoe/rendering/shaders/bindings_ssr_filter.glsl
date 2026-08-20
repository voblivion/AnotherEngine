#ifndef VOB_AOEGL_CORE_BINDINGS_SSR_FILTER_GLSL
#define VOB_AOEGL_CORE_BINDINGS_SSR_FILTER_GLSL

#include "core/bindings.glsl"

layout(binding = BINDING_TEXTURE_SSR_FILTER_SOURCE) uniform sampler2D uSsrFilter_Source;
layout(binding = BINDING_TEXTURE_SSR_FILTER_OPAQUE_DEPTH) uniform sampler2D uSsrFilter_OpaqueDepth;
layout(binding = BINDING_TEXTURE_SSR_FILTER_OPAQUE_NORMAL) uniform sampler2D uSsrFilter_OpaqueNormal;

#endif // #ifndef VOB_AOEGL_CORE_BINDINGS_SSR_FILTER_GLSL
