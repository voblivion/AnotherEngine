#ifndef VOB_AOEGL_CORE_BINDINGS_SSAO_GLSL
#define VOB_AOEGL_CORE_BINDINGS_SSAO_GLSL

#include "core/bindings.glsl"

layout(binding = BINDING_TEXTURE_SSAO_OPAQUE_GEOMETRIC_NORMAL) uniform sampler2D uSsao_OpaqueGeometricNormal;
layout(binding = BINDING_TEXTURE_SSAO_OPAQUE_DEPTH) uniform sampler2D uSsao_OpaqueDepth;
layout(binding = BINDING_TEXTURE_SSAO_LINEAR_DEPTH) uniform sampler2D uSsao_LinearDepth;
layout(binding = BINDING_TEXTURE_SSAO_RAW_OCCLUSION) uniform sampler2D uSsao_RawOcclusion;

#endif // #ifndef VOB_AOEGL_CORE_BINDINGS_SSAO_GLSL
