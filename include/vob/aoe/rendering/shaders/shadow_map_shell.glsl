#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"

#if USE_UV
in vec2 vUv;
#endif

#if USE_ALPHA_MASK
float getOpacity();
float getAlphaCutoff();
#endif


void main()
{
#if USE_ALPHA_MASK
    if (getOpacity() < getAlphaCutoff())
    {
        discard;
    }
#endif
}
