#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"

in vec3 vNormal;
#if USE_UV
in vec2 vUv;
#endif

layout(location = 0) out vec3 oGeometricNormal;

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

    oGeometricNormal = normalize(vNormal);
}
