#version 450 core

in vec3 vNormal;

layout(location = 0) out vec3 oGeometricNormal;


void main()
{
    oGeometricNormal = normalize(vNormal);
}
