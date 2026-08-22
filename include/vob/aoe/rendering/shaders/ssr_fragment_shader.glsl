#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssr.glsl"
#include "core/light_utils.glsl"

in vec2 vUv;
out vec4 oSsrColor;

vec3 getSkyColor(vec3 dir, float lod);


vec3 ReconstructViewPos2(vec2 uv, float depth)
{
    float linearDepth = uView.nearClip * uView.farClip / (uView.farClip - depth * (uView.farClip - uView.nearClip));

    float tanHalfFovX = 1.0 / uView.viewToClip[0][0];
    float tanHalfFovY = 1.0 / uView.viewToClip[1][1];

    vec2 ndc = uv * 2.0 - 1.0;
    float viewZ = -linearDepth;
    float viewX = ndc.x * tanHalfFovX * linearDepth;
    float viewY = ndc.y * tanHalfFovY * linearDepth;

    return vec3(viewX, viewY, viewZ);
}

float LinearizeDepth(float depth)
{
    return uView.nearClip * uView.farClip / (uView.farClip - depth * (uView.farClip - uView.nearClip));
}

// irradiance is a cosine-weighted integral, so it needs dividing by PI to read as radiance
vec3 skyIrradianceRadiance(vec3 direction)
{
    return max(uEvaluateSkyIrradiance(direction), vec3(0.0)) / 3.14159265358979;
}

// TEMPORARY: with the toggle on, every pixel reports why its walk ended instead of a colour, so the
// artefact names its own cause. Read it off "Ssr Raw" in the render inspector - "Ssr Color" is the
// prefiltered copy, and the prefilter blends flat debug colours together and blacks out anything
// with no normal.
//   grey    background            red     surface has no reflectance at all
//   cyan    reflects too little   magenta trust fade, no walk at all
//   green   hit                   blue    left the screen
//   yellow  reached the end       orange  ran out of steps
vec4 ssrResult(vec3 a_radiance, vec3 a_reasonColor)
{
    return vec4(uSsr.debugExitReason != 0 ? a_reasonColor : a_radiance, 1.0);
}

struct SsrRay
{
    bool valid;
    vec3 fallbackRadiance;
    vec3 fallbackReason;

    vec2 startPixel;
    vec2 pixelDir;
    float startDepth;
    float endDepth;

    vec3 skyColor;
    float screenTrust;
};

SsrRay setupSsrRay(vec2 a_uv)
{
    SsrRay ray;
    ray.valid = false;
    ray.fallbackRadiance = vec3(0.0);
    ray.startPixel = vec2(0.0);
    ray.pixelDir = vec2(0.0);
    ray.startDepth = 0.0;
    ray.endDepth = 0.0;
    ray.skyColor = vec3(0.0);
    ray.screenTrust = 0.0;

    // --- skip background ---
    // depth is hyperbolic, so a loose epsilon here covers a huge slice of the world: at a 0.1-1000
    // frustum, 0.9999 is only ~476m, and everything past it would be mistaken for empty background
    float depth = textureLod(uSsr_OpaqueDepth, a_uv, 0.0).r;
    if (1.0 - depth <= 1e-6)
    {
        ray.fallbackReason = vec3(0.25);
        return ray;
    }

    vec4 surface = textureLod(uSsr_OpaqueSurface, a_uv, 0.0);
    if (surface.r == 0.0)
    {
        ray.fallbackReason = vec3(1.0, 0.0, 0.0);
        return ray;
    }

    // --- reconstruct view-space position and normal ---
    vec3 viewPos = ReconstructViewPos2(a_uv, depth);
    vec3 normal  = normalize(mat3(uView.worldToView) * textureLod(uSsr_OpaqueNormal, a_uv, 0.0).xyz);

    // --- reflection ray in view space ---
    vec3 incident = normalize(viewPos - vec3(0.0)); // view-space: camera is at origin
    vec3 reflDir  = reflect(incident, normal);

    float roughness = surface.a;
    float NdotV = max(dot(normal, -incident), 0.0);
    vec2 envBrdf = envBrdfApprox(NdotV, roughness);
    vec3 specularWeight = surface.rgb * envBrdf.x + envBrdf.y;

    const float k_minSpecularWeight = 0.005;
    if (max(max(specularWeight.r, specularWeight.g), specularWeight.b) < k_minSpecularWeight)
    {
        ray.fallbackReason = vec3(0.0, 1.0, 1.0);
        return ray;
    }

    // rays travelling back toward the camera need information the screen does not hold, and one
    // mirror ray only represents a narrow lobe - the wider it gets, the less a single sample says
    // about it, so lean on the sky's prefiltered version instead
    ray.screenTrust = smoothstep(0.25, 0.0, reflDir.z) * (1.0 - smoothstep(0.45, 0.85, roughness));

    vec3 reflDirWorld = normalize(mat3(uView.viewToWorld) * reflDir);

    // both factors are known before marching, so nothing the march finds could change the answer
    if (ray.screenTrust <= 0.0)
    {
        ray.fallbackRadiance = skyIrradianceRadiance(reflDirWorld);
        ray.fallbackReason = vec3(1.0, 0.0, 1.0);
        return ray;
    }

    // at the roughest end the lobe is wide enough that the SH hemisphere stands in for the sky, at
    // nine madds instead of a procedural sky evaluation
    float skyBlend = smoothstep(0.45, 0.85, roughness);
    ray.skyColor = getSkyColor(reflDirWorld, clamp(roughness * 2.0, 0.0, 1.0));
    if (skyBlend > 0.0)
    {
        ray.skyColor = mix(ray.skyColor, skyIrradianceRadiance(reflDirWorld), skyBlend);
    }

    // a ray reaching past the near plane projects through a negative w and lands nowhere useful, so
    // cut it at the near plane first
    vec3 endViewPos = viewPos + reflDir * uSsr.maxRange;
    if (endViewPos.z > -uView.nearClip)
    {
        float nearT = (-uView.nearClip - viewPos.z) / (endViewPos.z - viewPos.z);
        endViewPos = viewPos + (endViewPos - viewPos) * clamp(nearT, 0.0, 1.0);
    }

    // past the far plane the projected depth exceeds 1.0, and 1.0 is what the depth buffer holds
    // where nothing was drawn - the walk would read that as geometry and "hit" the background
    if (endViewPos.z < -uView.farClip)
    {
        float farT = (-uView.farClip - viewPos.z) / (endViewPos.z - viewPos.z);
        endViewPos = viewPos + (endViewPos - viewPos) * clamp(farT, 0.0, 1.0);
    }

    vec4 reflEndClip = uView.viewToClip * vec4(endViewPos, 1.0);
    vec3 reflEndNDC  = reflEndClip.xyz / reflEndClip.w;

    vec4 startClip = uView.viewToClip * vec4(viewPos, 1.0);
    vec3 startNDC  = startClip.xyz / startClip.w;

    // a projective transform maps a line to a line, so pixel position and depth are both linear in
    // t along the projected segment - the whole walk is affine in t
    vec2 hiZSize = vec2(textureSize(uSsr_HiZDepth, 0));
    ray.startPixel = (startNDC.xy * 0.5 + 0.5) * hiZSize;
    ray.pixelDir = (reflEndNDC.xy * 0.5 + 0.5) * hiZSize - ray.startPixel;
    ray.startDepth = startNDC.z * 0.5 + 0.5;
    ray.endDepth = reflEndNDC.z * 0.5 + 0.5;

    ray.valid = true;
    return ray;
}

// One stretch of ray spent behind an occluder: where it went behind, where it came back out, and how
// deep past the surface it was at each end. Everything the absorption rule might want is here, so
// that rule can be rewritten without touching the traversal.
struct SsrOccluderSpan
{
    float entryRayDepth;      // ray depth when it went behind, in depth-buffer units
    float entrySurfaceDepth;  // the surface it went behind
    float exitRayDepth;       // ray depth when it re-emerged
    float exitSurfaceDepth;   // the surface it came out in front of
    vec2 entryPixel;
    vec2 exitPixel;
    float entryT;
    float exitT;
    bool reEmerged;           // false when the walk ended still behind the occluder
};

// How much of the ray survives an occluder: 0 fully blocked, 1 passes through untouched.
// Penetration is how far past the surface the ray got, as a fraction of that surface's depth - a
// ray still hugging the surface is inside the thing, one far past it went by behind.
// How far past the surface the ray got, as a fraction of that surface's distance - 0.05 means the
// ray is a twentieth of the way further out than the thing it went behind. Depths are linearised
// first: the raw buffer is hyperbolic, so a difference taken in it is not a distance and means
// something different at every range.
float ssrOccluderPenetration(SsrOccluderSpan a_span)
{
    // both ends are measured against the surface the ray went BEHIND. The surface it re-emerges in
    // front of is a different thing entirely - usually whatever lies beyond the occluder - and
    // measuring against that gives a large negative number rather than a penetration
    float surface = LinearizeDepth(a_span.entrySurfaceDepth);
    float entryRay = LinearizeDepth(a_span.entryRayDepth);
    float exitRay = LinearizeDepth(a_span.exitRayDepth);

    float entryPenetration = max((entryRay - surface) / max(surface, 1e-6), 0.0);
    float exitPenetration = max((exitRay - surface) / max(surface, 1e-6), 0.0);

    return entryPenetration;
    return sqrt(entryPenetration * exitPenetration);
    return entryPenetration; //  0.5 * (entryPenetration + exitPenetration);
}

float ssrOccluderTransmittance(SsrOccluderSpan a_span)
{
    // a stretch that never came back out is not automatically solid - the walk may simply have run
    // out of screen or range while still behind it. Its exit fields hold the entry values, so this
    // reduces to entry penetration and the same rule decides.
    return smoothstep(
        uSsr.penetrationBlockedRatio, uSsr.penetrationThroughRatio, ssrOccluderPenetration(a_span));
}

struct SsrWalk
{
    vec3 blockedColor; // what the occluders along the way absorbed, already weighted
    float transmittance;
    vec3 exitReason;
    int probeLevel; // finest level whose tested cell contained the probe, -1 if it never did
    bool metOccluder;
    float firstPenetration; // of the first occluder met, for tuning the thresholds against
};

SsrWalk walkSsrRay(SsrRay a_ray, ivec2 a_probePixel)
{
    SsrWalk walk;
    walk.blockedColor = vec3(0.0);
    walk.transmittance = 1.0;
    walk.exitReason = vec3(1.0, 0.5, 0.0); // ran out of steps unless something else ends the walk
    walk.probeLevel = -1;
    walk.metOccluder = false;
    walk.firstPenetration = 0.0;

    SsrOccluderSpan span;
    span.reEmerged = false;
    bool occluded = false;
    vec3 entryColor = vec3(0.0);
    float entryConfidence = 0.0;

    vec2 hiZSize = vec2(textureSize(uSsr_HiZDepth, 0));
    vec2 probePoint = vec2(a_probePixel) + 0.5;

    // a cell is only left through the boundary the ray is heading toward, and a zero component never
    // reaches one - a huge t keeps it from winning the min
    vec2 stepSign = vec2(a_ray.pixelDir.x >= 0.0 ? 1.0 : 0.0, a_ray.pixelDir.y >= 0.0 ? 1.0 : 0.0);
    vec2 invPixelDir = vec2(
        abs(a_ray.pixelDir.x) > 1e-6 ? 1.0 / a_ray.pixelDir.x : 0.0
        , abs(a_ray.pixelDir.y) > 1e-6 ? 1.0 / a_ray.pixelDir.y : 0.0);

    // how much t buys one pixel of real travel, taken on the axis the ray actually moves along: a
    // near-vertical ray has a huge t-per-pixel sideways, and nudging by that would leap whole cells
    float tPerPixel = 1.0 / max(max(abs(a_ray.pixelDir.x), abs(a_ray.pixelDir.y)), 1e-6);

    int maxLevel = textureQueryLevels(uSsr_HiZDepth) - 1;
    int level = 0;
    float t = 0.0;

    for (int i = 0; i < uSsr.stepCount; ++i)
    {
        vec2 pixel = a_ray.startPixel + a_ray.pixelDir * t;
        if (pixel.x < 0.0 || pixel.y < 0.0 || pixel.x >= hiZSize.x || pixel.y >= hiZSize.y)
        {
            walk.exitReason = vec3(0.0, 0.0, 1.0);
            break;
        }

        if (t >= 1.0)
        {
            walk.exitReason = vec3(1.0, 1.0, 0.0);
            break;
        }

        // advance to where the ray leaves this cell, never further: whatever is skipped has been
        // proven empty, so there is never anything to backtrack over
        float cellSize = float(1 << level);
        vec2 cellMin = floor(pixel / cellSize) * cellSize;
        vec2 boundary = cellMin + stepSign * cellSize;
        vec2 boundaryT = (boundary - a_ray.startPixel) * invPixelDir;
        boundaryT = mix(vec2(1e9), boundaryT, notEqual(invPixelDir, vec2(0.0)));

        // land a hundredth of a pixel past the boundary: enough to be in the next cell, small enough
        // that no geometry hides in what it skips. The floor guarantees the walk moves: rebuilding
        // the pixel from t can round back inside the cell just left, and the arithmetic being
        // deterministic, that step would then repeat unchanged until the budget ran out
        float minStep = max(0.01 * tPerPixel, t * 1e-5);
        float exitT = min(max(min(boundaryT.x, boundaryT.y) + minStep, t + minStep), 1.0);

        // the probe records every cell this walk tested, keeping the finest - a descent nests a
        // smaller cell inside the one that failed, and the smaller one says more
        if (a_probePixel.x >= 0
            && all(greaterThanEqual(probePoint, cellMin))
            && all(lessThan(probePoint, cellMin + cellSize)))
        {
            walk.probeLevel = walk.probeLevel < 0 ? level : min(walk.probeLevel, level);
        }

        // mip sizes floor, so the last row and column of cells run past the level's texel count
        ivec2 cellCoord = min(ivec2(pixel / cellSize), textureSize(uSsr_HiZDepth, level) - 1);
        vec2 cell = texelFetch(uSsr_HiZDepth, cellCoord, level).rg; // r nearest, g farthest
        float entryDepth = mix(a_ray.startDepth, a_ray.endDepth, t);
        float exitDepth = mix(a_ray.startDepth, a_ray.endDepth, exitT);

        vec2 exitPixel = a_ray.startPixel + a_ray.pixelDir * exitT;
        bool enteredNewCoarseCell = level < maxLevel
            && floor(exitPixel / (cellSize * 2.0)) != floor(pixel / (cellSize * 2.0));

        // depth is linear in t, so the ray's extremes while crossing this cell are at its two ends.
        // In front of everything means the cell can be skipped; behind everything means the same
        // while occluded, which is what lets an occluded stretch be crossed in big strides instead
        // of one pixel at a time.
        // A cell still at the cleared depth holds no geometry at all, and the colour target has not
        // been given a sky yet at this point in the frame, so hitting one would reflect black.
        // The first cell is the one the ray is leaving, where its depth equals the surface's by
        // construction - testing it would make every surface reflect itself
        bool cellSkipped = occluded
            ? min(entryDepth, exitDepth) >= cell.g
            : (i == 0 || cell.r >= 1.0 || max(entryDepth, exitDepth) <= cell.r);

        if (cellSkipped)
        {
            t = exitT;
            level += enteredNewCoarseCell ? 1 : 0;
            continue;
        }

        if (level > 0)
        {
            // the cell may hold something, so look closer without moving - nothing to undo
            --level;
            continue;
        }

        vec2 cellUv = pixel / hiZSize;
        t = exitT;

        if (!occluded)
        {
            // went behind this surface: remember where, and keep walking to find out for how long
            occluded = true;
            span.entryRayDepth = max(entryDepth, exitDepth);
            span.entrySurfaceDepth = cell.r;
            span.entryPixel = pixel;
            span.entryT = t;
            span.reEmerged = false;

            // so a stretch that never comes back out still reads as its entry penetration
            span.exitRayDepth = span.entryRayDepth;
            span.exitSurfaceDepth = span.entrySurfaceDepth;
            span.exitPixel = span.entryPixel;
            span.exitT = span.entryT;

            entryColor = textureLod(uSsr_DirectOpaqueColor, cellUv, 0.0).rgb;
            vec2 borderDist = min(cellUv, 1.0 - cellUv);
            entryConfidence = smoothstep(0.0, 0.1, min(borderDist.x, borderDist.y));
            continue;
        }

        // came back out in front: the stretch is complete, so it can be judged as a whole
        span.exitRayDepth = min(entryDepth, exitDepth);
        span.exitSurfaceDepth = cell.r;
        span.exitPixel = pixel;
        span.exitT = t;
        span.reEmerged = true;
        occluded = false;

        if (!walk.metOccluder)
        {
            walk.metOccluder = true;
            walk.firstPenetration = ssrOccluderPenetration(span);
        }

        // an occluder near the screen edge is only partly known, so it absorbs proportionally less
        float absorbed = (1.0 - ssrOccluderTransmittance(span)) * entryConfidence;
        walk.blockedColor += walk.transmittance * absorbed * entryColor;
        walk.transmittance *= 1.0 - absorbed;

        if (walk.transmittance < 0.02)
        {
            walk.exitReason = vec3(0.0, 1.0, 0.0);
            break;
        }
    }

    // the walk ended without ever coming back out, so the occluder covers everything beyond
    if (occluded)
    {
        span.reEmerged = false;
        if (!walk.metOccluder)
        {
            walk.metOccluder = true;
            walk.firstPenetration = ssrOccluderPenetration(span);
        }

        float absorbed = (1.0 - ssrOccluderTransmittance(span)) * entryConfidence;
        walk.blockedColor += walk.transmittance * absorbed * entryColor;
        walk.transmittance *= 1.0 - absorbed;
        walk.exitReason = vec3(0.0, 1.0, 0.0);
    }

    return walk;
}

// TEMPORARY: trace one chosen pixel's ray across the whole screen. Every fragment re-walks that ray
// and asks whether it was inside any cell the walk tested; the colour runs yellow at level 0 to
// green at the coarsest, so stride reads directly off it. Cells are filled dim and the ray line
// itself is drawn bright. Read it off "Ssr Raw" in the render inspector.
bool debugRayOverlay(out vec3 a_color)
{
    vec2 hiZSize = vec2(textureSize(uSsr_HiZDepth, 0));
    vec2 fragPixel = gl_FragCoord.xy;
    vec2 debugPixel = vec2(uSsr.debugRayPixel) + 0.5;

    // always show where the chosen pixel is, so an empty screen means "no ray here", not "broken"
    if (distance(fragPixel, debugPixel) < 2.0)
    {
        a_color = vec3(1.0);
        return true;
    }

    SsrRay ray = setupSsrRay(debugPixel / hiZSize);
    if (!ray.valid)
    {
        return false;
    }

    SsrWalk walk = walkSsrRay(ray, ivec2(fragPixel));
    if (walk.probeLevel < 0)
    {
        return false;
    }

    int maxLevel = textureQueryLevels(uSsr_HiZDepth) - 1;
    float levelRatio = float(walk.probeLevel) / float(max(maxLevel, 1));
    a_color = vec3(1.0 - levelRatio, 1.0, 0.0);

    float rayLength = length(ray.pixelDir);
    vec2 rayNormal = rayLength > 0.0 ? vec2(-ray.pixelDir.y, ray.pixelDir.x) / rayLength : vec2(0.0);
    bool onRayLine = abs(dot(fragPixel - ray.startPixel, rayNormal)) < 0.7;

    a_color *= onRayLine ? 1.0 : 0.3;
    return true;
}

void main()
{
    SsrRay ray = setupSsrRay(vUv);

    vec3 radiance = ray.fallbackRadiance;
    vec3 reason = ray.fallbackReason;
    if (ray.valid)
    {
        SsrWalk walk = walkSsrRay(ray, ivec2(-1));

        // TEMPORARY: the first occluder's penetration against the two thresholds, so the tuning
        // window can be seen rather than guessed. Red at or below r0 (fully blocked), green at or
        // above r1 (fully through), dim blue where the ray met no occluder at all.
        if (uSsr.debugPenetration != 0)
        {
            vec3 penetrationColor = vec3(0.0, 0.0, 0.25);
            if (walk.metOccluder)
            {
                float k = smoothstep(
                    uSsr.penetrationBlockedRatio, uSsr.penetrationThroughRatio, walk.firstPenetration);
                penetrationColor = vec3(1.0 - k, k, 0.0);
            }

            oSsrColor = vec4(penetrationColor, 1.0);
            return;
        }

        // whatever survived the occluders reaches the sky; all of it is screen-space, so all of it
        // answers to the screen trust
        vec3 screenColor = walk.blockedColor + walk.transmittance * ray.skyColor;
        radiance = mix(ray.skyColor, screenColor, ray.screenTrust);
        reason = walk.exitReason;
    }

    vec3 overlayColor;
    if (uSsr.debugRay != 0 && debugRayOverlay(overlayColor))
    {
        oSsrColor = vec4(overlayColor, 1.0);
        return;
    }

    // the hit is screen-space, so it answers to the screen trust as well as its own border fade
    oSsrColor = ssrResult(radiance, reason);
}
