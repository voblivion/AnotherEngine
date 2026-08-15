#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"

in vec3 vPosition;
in vec2 vUv;
in mat3 vTBN;

layout(location = 0) out vec3 oColor;
layout(location = 1) out vec3 oNormal;
layout(location = 2) out vec4 oSurface;

struct OpaqueOutputs
{
    vec3 color;
    vec3 normal;
    vec4 surface;
#if USE_ALPHA_TEST
    float opacity;
#endif
};

OpaqueOutputs getOpaqueOutputs();
#if USE_ALPHA_TEST
float getOpacityDiscardThreshold();
#endif

void main()
{
    OpaqueOutputs outputs = getOpaqueOutputs();

#if USE_ALPHA_TEST
    if (outputs.opacity < getOpacityDiscardThreshold())
    {
        discard;
    }
#endif

    oColor = outputs.color;
    oNormal = outputs.normal;
    oSurface = outputs.surface;
}
