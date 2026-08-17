#include "core/bindings.glsl"

layout(std140, binding = BINDING_UBO_MATERIAL) uniform BasicOpaqueParams
{
    vec4 uAlbedo;
    float uMetallic;
    float uRoughness;
};

#if USE_SHADING
#include "core/shading_utils.glsl"

OpaqueOutputs getOpaqueOutputs()
{
    vec3 normal = normalize(vTBN * vec3(0.0, 0.0, 1.0));

    return uShadeStandardOpaqueSurface(gl_FragCoord, vPosition, normal, uAlbedo.xyz, uMetallic, uRoughness, 0.5, 1.0);
}
#endif
