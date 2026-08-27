#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssao.glsl"
#include "core/math_utils.glsl"

in vec2 vUv;
layout(location = 0) out float oAmbientOcclusion;


// Every aligned 4x4 block holds each of the 16 directions once, so a denoiser covering the block
// sums a complete set and the directional bias cancels instead of merely blurring.
float sliceRotationNoise(ivec2 a_pixel)
{
    return (1.0 / 16.0) * float((((a_pixel.x + a_pixel.y) & 3) << 2) + (a_pixel.x & 3));
}

float stepOffsetNoise(ivec2 a_pixel)
{
    return 0.25 * float((a_pixel.y - a_pixel.x) & 3);
}

vec3 viewPositionAt(vec2 uv)
{
    vec2 ndc = uv * 2.0 - 1.0;
    vec3 viewRay = vec3(ndc.x / uView.viewToClip[0][0], ndc.y / uView.viewToClip[1][1], -1.0);
    return viewRay * texture(uSsao_LinearDepth, uv).r;
}

// Searches one side of a slice for the highest horizon, expressed as cos(angle to the view vector).
float findHorizonCosine(
    vec2 a_uv
    , vec3 a_position
    , vec3 a_viewDir
    , vec2 a_direction
    , float a_radiusPixels
    , float a_radiusWorld
    , vec2 a_resolution
    , float a_noise)
{
    float horizonCosine = -1.0;
    for (int step = 0; step < uSsao.stepCount; ++step)
    {
        float t = (float(step) + a_noise) / float(uSsao.stepCount);
        float distancePixels = mix(1.0, max(a_radiusPixels, 1.0), t * t);
        vec3 delta = viewPositionAt(a_uv + a_direction * distancePixels / a_resolution) - a_position;

        float distance = length(delta);
        float sampleCosine = dot(delta / max(distance, 1e-6), a_viewDir);

        float fadeBegin = a_radiusWorld * uSsao.falloffStart;
        float falloff = clamp((distance - fadeBegin) / max(a_radiusWorld - fadeBegin, 1e-6), 0.0, 1.0);
        horizonCosine = max(horizonCosine, mix(sampleCosine, horizonCosine, falloff));
    }
    return horizonCosine;
}

// Cosine-weighted visibility of the arc between two horizons, around the projected normal.
float integrateArc(float a_horizon0, float a_horizon1, float a_normalAngle, float a_normalLength)
{
    float h0 = a_normalAngle + max(-acos(a_horizon0) - a_normalAngle, -k_halfPi);
    float h1 = a_normalAngle + min(acos(a_horizon1) - a_normalAngle, k_halfPi);

    float cosNormalAngle = cos(a_normalAngle);
    float sinNormalAngle = sin(a_normalAngle);

    return 0.25 * a_normalLength * (
        (-cos(2.0 * h0 - a_normalAngle) + cosNormalAngle + 2.0 * h0 * sinNormalAngle)
        + (-cos(2.0 * h1 - a_normalAngle) + cosNormalAngle + 2.0 * h1 * sinNormalAngle));
}

void main()
{
    vec3 position = viewPositionAt(vUv);
    vec3 normal = normalize(mat3(uView.worldToView) * texture(uSsao_OpaqueGeometricNormal, vUv).rgb);
    vec3 viewDir = normalize(-position);

    vec2 resolution = vec2(textureSize(uSsao_LinearDepth, 0));
    float pixelsPerWorldUnit = 0.5 * uView.viewToClip[0][0] * resolution.x / max(-position.z, 1e-4);
    float radiusPixels = min(uSsao.radius * pixelsPerWorldUnit, uSsao.maxRadiusPixels);
    float radiusWorld = radiusPixels / max(pixelsPerWorldUnit, 1e-6);

    ivec2 pixel = ivec2(gl_FragCoord.xy);
    float sliceNoise = sliceRotationNoise(pixel);
    float stepNoise = stepOffsetNoise(pixel);

    float visibility = 0.0;
    for (int slice = 0; slice < uSsao.sliceCount; ++slice)
    {
        float sliceAngle = (float(slice) + sliceNoise) * k_pi / float(uSsao.sliceCount);
        vec2 sliceDir = vec2(cos(sliceAngle), sin(sliceAngle));

        vec3 sliceDirView = vec3(sliceDir, 0.0);
        vec3 sliceAxis = normalize(cross(sliceDirView, viewDir));
        vec3 projectedNormal = normal - sliceAxis * dot(normal, sliceAxis);

        float projectedNormalLength = length(projectedNormal);
        if (projectedNormalLength < 1e-4)
        {
            continue;
        }

        vec3 sliceTangent = normalize(sliceDirView - viewDir * dot(sliceDirView, viewDir));
        float normalAngle = atan(
            dot(projectedNormal, sliceTangent), dot(projectedNormal, viewDir));

        float horizon0 = findHorizonCosine(
            vUv, position, viewDir, -sliceDir, radiusPixels, radiusWorld, resolution, stepNoise);
        float horizon1 = findHorizonCosine(
            vUv, position, viewDir, sliceDir, radiusPixels, radiusWorld, resolution, stepNoise);

        visibility += integrateArc(horizon0, horizon1, normalAngle, projectedNormalLength);
    }

    visibility = clamp(visibility / float(uSsao.sliceCount), 0.0, 1.0);
    oAmbientOcclusion = pow(visibility, uSsao.intensity);
}
