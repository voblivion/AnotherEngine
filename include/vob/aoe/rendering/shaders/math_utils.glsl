#ifndef VOB_AOEGL_MATH_UTILS_GLSL
#define VOB_AOEGL_MATH_UTILS_GLSL

const float k_pi = 3.14159265359;
const float k_halfPi = 1.57079632679;

float linearizeDepth(float depth, float near, float far)
{
    return 2.0 * near * far / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

vec3 intersectViewDepthPlane(vec3 dir, float depth)
{
    float t = -depth / dir.z;
    return t * dir;
}


#endif // #ifndef VOB_AOEGL_MATH_UTILS_GLSL