#version 450 core

#extension GL_NV_gpu_shader5 : enable

#include "core/bindings_ssr_filter.glsl"


out vec2 oDepth;

// r is the nearest surface in the cell, g the farthest. The near one answers "is the ray in front of
// everything here", which is what lets a cell be skipped; the far one answers "is the ray behind
// everything here", which is what will let an occluded stretch be skipped.
void main()
{
    ivec2 sourceSize = textureSize(uSsrFilter_Source, 0);

    // level 0 is seeded straight from the depth buffer, which is the one pass where source and
    // destination share a resolution
    if (sourceSize == uTarget.resolution)
    {
        oDepth = vec2(texelFetch(uSsrFilter_Source, ivec2(gl_FragCoord.xy), 0).r);
        return;
    }

    ivec2 base = ivec2(gl_FragCoord.xy) * 2;

    vec2 d = texelFetch(uSsrFilter_Source, min(base, sourceSize - 1), 0).rg;
    vec2 d10 = texelFetch(uSsrFilter_Source, min(base + ivec2(1, 0), sourceSize - 1), 0).rg;
    vec2 d01 = texelFetch(uSsrFilter_Source, min(base + ivec2(0, 1), sourceSize - 1), 0).rg;
    vec2 d11 = texelFetch(uSsrFilter_Source, min(base + ivec2(1, 1), sourceSize - 1), 0).rg;
    d = vec2(min(min(d.r, d10.r), min(d01.r, d11.r)), max(max(d.g, d10.g), max(d01.g, d11.g)));

    // an odd source leaves a row or column that no 2x2 block covers, and dropping it would let the
    // traversal skip a cell that does contain geometry
    if ((sourceSize.x & 1) != 0 && base.x + 2 == sourceSize.x - 1)
    {
        vec2 e0 = texelFetch(uSsrFilter_Source, ivec2(base.x + 2, base.y), 0).rg;
        vec2 e1 = texelFetch(uSsrFilter_Source, min(ivec2(base.x + 2, base.y + 1), sourceSize - 1), 0).rg;
        d = vec2(min(d.r, min(e0.r, e1.r)), max(d.g, max(e0.g, e1.g)));
    }
    if ((sourceSize.y & 1) != 0 && base.y + 2 == sourceSize.y - 1)
    {
        vec2 e0 = texelFetch(uSsrFilter_Source, ivec2(base.x, base.y + 2), 0).rg;
        vec2 e1 = texelFetch(uSsrFilter_Source, min(ivec2(base.x + 1, base.y + 2), sourceSize - 1), 0).rg;
        d = vec2(min(d.r, min(e0.r, e1.r)), max(d.g, max(e0.g, e1.g)));
    }

    oDepth = d;
}
