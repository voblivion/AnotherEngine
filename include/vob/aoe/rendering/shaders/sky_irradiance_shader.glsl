#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings.glsl"
#include "core/math_utils.glsl"

layout(local_size_x = SKY_IRRADIANCE_SAMPLE_COUNT) in;

vec3 getSkyColor(vec3 dir, float lod);

shared vec3 s_directions[SKY_IRRADIANCE_SAMPLE_COUNT];
shared vec3 s_radiances[SKY_IRRADIANCE_SAMPLE_COUNT];

void main()
{
    float sampleCount = float(SKY_IRRADIANCE_SAMPLE_COUNT);
    float index = float(gl_LocalInvocationIndex) + 0.5;

    float cosPhi = 1.0 - 2.0 * index / sampleCount;
    float sinPhi = sqrt(max(1.0 - cosPhi * cosPhi, 0.0));
    float theta = 3.14159265358979 * (1.0 + sqrt(5.0)) * index;
    vec3 dir = vec3(cos(theta) * sinPhi, cosPhi, sin(theta) * sinPhi);

    s_directions[gl_LocalInvocationIndex] = dir;
    s_radiances[gl_LocalInvocationIndex] = getSkyColor(dir, 1.0);

    barrier();

    if (gl_LocalInvocationIndex != 0)
    {
        return;
    }

    vec3 coefficients[SKY_IRRADIANCE_COEFFICIENT_COUNT];
    for (int c = 0; c < SKY_IRRADIANCE_COEFFICIENT_COUNT; ++c)
    {
        coefficients[c] = vec3(0.0);
    }

    for (int s = 0; s < SKY_IRRADIANCE_SAMPLE_COUNT; ++s)
    {
        vec3 d = s_directions[s];
        vec3 radiance = s_radiances[s];

        coefficients[0] += radiance * 0.282095;
        coefficients[1] += radiance * 0.488603 * d.y;
        coefficients[2] += radiance * 0.488603 * d.z;
        coefficients[3] += radiance * 0.488603 * d.x;
        coefficients[4] += radiance * 1.092548 * d.x * d.y;
        coefficients[5] += radiance * 1.092548 * d.y * d.z;
        coefficients[6] += radiance * 0.315392 * (3.0 * d.z * d.z - 1.0);
        coefficients[7] += radiance * 1.092548 * d.x * d.z;
        coefficients[8] += radiance * 0.546274 * (d.x * d.x - d.y * d.y);
    }

    float sampleWeight = 4.0 * 3.14159265358979 / sampleCount;
    float bandWeights[SKY_IRRADIANCE_COEFFICIENT_COUNT] = float[](
        3.141593, 2.094395, 2.094395, 2.094395, 0.785398, 0.785398, 0.785398, 0.785398, 0.785398);

    for (int c = 0; c < SKY_IRRADIANCE_COEFFICIENT_COUNT; ++c)
    {
        uSkyIrradiance[c] = vec4(coefficients[c] * sampleWeight * bandWeights[c], 0.0);
    }
}
