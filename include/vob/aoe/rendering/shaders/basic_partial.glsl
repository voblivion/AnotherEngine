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
    float reflectance = 0.5;

    OpaqueOutputs outputs;
    outputs.color = uEvaluateLights(gl_FragCoord, vPosition, normal, uAlbedo.xyz, uMetallic, uRoughness, reflectance);
    outputs.normal = normal;
    outputs.surface = vec4(mix(vec3(0.16 * reflectance * reflectance), uAlbedo.xyz, uMetallic), uRoughness);

    return outputs;
}
#endif
