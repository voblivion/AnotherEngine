#include "core/bindings.glsl"

#if USE_SHADING
#include "core/shading_utils.glsl"

OpaqueOutputs getOpaqueOutputs()
{
    vec3 normal = normalize(vTBN * vec3(0.0, 0.0, 1.0));

    OpaqueOutputs outputs = uShadeStandardOpaqueSurface(
        gl_FragCoord, vPosition, normal, uMaterial.albedo.xyz, uMaterial.metallic, uMaterial.roughness, 0.5, 1.0);

    outputs.color += uMaterial.emissive.rgb * uMaterial.emissive.a;

    return outputs;
}
#endif
