#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_opaque_composition.glsl"
#include "core/light_utils.glsl"

in vec2 vUv;
out vec4 oColor;


void main()
{
    vec4 surface = texture(uOpaqueComposition_OpaqueSurface, vUv);
    vec3 f0 = surface.rgb;
    float roughness = surface.a;

    vec3 reflection = vec3(0.0);
    if (uSsr.isEnabled != 0 && f0 != vec3(0.0))
    {
        float pixelsPerRadian = 0.5 * uView.viewToClip[0][0]
            * float(textureSize(uOpaqueComposition_SsrColor, 0).x);
        float lod = log2(max(roughness * roughness * pixelsPerRadian, 1.0));
        vec3 radiance = textureLod(uOpaqueComposition_SsrColor, vUv, lod).rgb;

        vec4 viewH = uView.clipToView * vec4(vUv * 2.0 - 1.0, 1.0, 1.0);
        vec3 viewDir = normalize(viewH.xyz / viewH.w);
        vec3 normal = normalize(mat3(uView.worldToView) * texture(uOpaqueComposition_OpaqueNormal, vUv).xyz);
        float NdotV = max(dot(normal, -viewDir), 0.0);

        vec2 envBrdf = envBrdfApprox(NdotV, roughness);
        reflection = radiance * (f0 * envBrdf.x + envBrdf.y);
    }

    oColor = vec4(texture(uOpaqueComposition_DirectOpaqueColor, vUv).rgb + reflection, 1.0);
}
