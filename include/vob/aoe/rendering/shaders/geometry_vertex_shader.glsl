#version 450 core
#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"

layout(location = 0) in vec3 aPosition;
#if USE_SHADING || USE_NORMAL
layout(location = 1) in vec3 aNormal;
#endif
#if USE_SHADING
layout(location = 2) in vec2 aUv;
layout(location = 3) in vec3 aTangent;
#endif
#if USE_RIG
layout(location = 4) in ivec4 aBoneIndices;
layout(location = 5) in vec4 aBoneWeights;
#endif

#if USE_SHADING
out vec3 vPosition;
out vec2 vUv;
out mat3 vTBN;
#endif
#if USE_NORMAL
out vec3 vNormal;
#endif

void main()
{
#if USE_RIG
    mat4 skin = uRig.bones[aBoneIndices.x] * aBoneWeights.x
        + uRig.bones[aBoneIndices.y] * aBoneWeights.y
        + uRig.bones[aBoneIndices.z] * aBoneWeights.z
        + uRig.bones[aBoneIndices.w] * aBoneWeights.w;
    
    mat4 skinToWorld = uModel.modelToWorld * skin;
    vec4 position = skinToWorld * vec4(aPosition, 1.0);
#if USE_SHADING || USE_NORMAL
    mat3 normalMatrix = mat3(skinToWorld);
#endif
#else
    vec4 position = uModel.modelToWorld * vec4(aPosition, 1.0);
#if USE_SHADING || USE_NORMAL
    mat3 normalMatrix = mat3(uModel.modelToWorld);
#endif
#endif

#if USE_SHADING
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    vPosition = position.xyz;
    vUv = aUv;
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vTBN = mat3(T, B, N);
#endif
#if USE_NORMAL
    vNormal = normalize(normalMatrix * aNormal);
#endif
    gl_Position = uView.worldToClip * position;
}
