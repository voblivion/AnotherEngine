#ifndef VOB_AOEGL_NOISE_UTILS_GLSL
#define VOB_AOEGL_NOISE_UTILS_GLSL

vec3 hash3(vec3 pos)
{
    vec3 p = fract(pos * vec3(0.1031, 0.1030, 0.0973));
    p += dot(p, p.yxz + 33.33);
    return fract((p.xxy + p.yxx) * p.zyx);
}

float voronoiDistance(vec3 integral, vec3 fractional, vec3 cellOffset)
{
    vec3 h = hash3(integral + cellOffset);
    vec3 dir = h + cellOffset - fractional;
    return dot(dir, dir);
}

float voronoiNoise(vec3 pos)
{
    vec3 integral = floor(pos);
    vec3 fractional = fract(pos);
    float minDistance = 10.0;
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            for (int k = -1; k <= 1; ++k)
            {
                minDistance = min(minDistance, voronoiDistance(integral, fractional, vec3(i, j, k)));
            }
        }
    }
    return minDistance;
}

float hash(vec3 pos)
{
    vec3 p = fract(pos * vec3(0.1031, 0.1030, 0.0973));
    p += dot(p, p.yxz + 33.33);
    return fract((p.x + p.y) * p.z);
}

float valueNoise(vec3 pos)
{
    vec3 i = floor(pos);
    vec3 f = fract(pos);
    vec3 u = f * f * (3.0 - 2.0 * f);
    
    return mix(
            mix(
                mix(hash(i + vec3(0, 0, 0)), hash(i + vec3(1, 0, 0)), u.x),
                mix(hash(i + vec3(0, 1, 0)), hash(i + vec3(1, 1, 0)), u.x),
                u.y),
            mix(
                mix(hash(i + vec3(0, 0, 1)), hash(i + vec3(1, 0, 1)), u.x),
                mix(hash(i + vec3(0, 1, 1)), hash(i + vec3(1, 1, 1)), u.x),
                u.y),
            u.z);
}

vec3 grad(vec3 pos)
{
    vec3 h = hash3(pos) * 2.0 - 1.0;
    return normalize(h);
}

float perlinNoise(vec3 pos)
{
    vec3 i = floor(pos);
    vec3 f = fract(pos);
    vec3 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    
    return mix(
        mix(mix(dot(grad(i + vec3(0,0,0)), f - vec3(0,0,0)),
                dot(grad(i + vec3(1,0,0)), f - vec3(1,0,0)), u.x),
            mix(dot(grad(i + vec3(0,1,0)), f - vec3(0,1,0)),
                dot(grad(i + vec3(1,1,0)), f - vec3(1,1,0)), u.x), u.y),
        mix(mix(dot(grad(i + vec3(0,0,1)), f - vec3(0,0,1)),
                dot(grad(i + vec3(1,0,1)), f - vec3(1,0,1)), u.x),
            mix(dot(grad(i + vec3(0,1,1)), f - vec3(0,1,1)),
                dot(grad(i + vec3(1,1,1)), f - vec3(1,1,1)), u.x), u.y),
        u.z) * 0.5 + 0.5;
}


#endif // #ifndef VOB_AOEGL_NOISE_UTILS_GLSL