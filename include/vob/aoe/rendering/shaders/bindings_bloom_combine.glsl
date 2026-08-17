#ifndef VOB_AOEGL_CORE_BINDINGS_BLOOM_COMBINE_GLSL
#define VOB_AOEGL_CORE_BINDINGS_BLOOM_COMBINE_GLSL

#include "core/bindings.glsl"

layout(binding = BINDING_TEXTURE_BLOOM_COMBINE_SCENE) uniform sampler2D uBloomCombine_Scene;
layout(binding = BINDING_TEXTURE_BLOOM_COMBINE_BLOOM) uniform sampler2D uBloomCombine_Bloom;

#endif // #ifndef VOB_AOEGL_CORE_BINDINGS_BLOOM_COMBINE_GLSL
