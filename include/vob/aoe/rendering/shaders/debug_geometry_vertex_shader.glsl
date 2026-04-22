#version 450 core

#extension GL_NV_gpu_shader5 : enable

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;

#include "core/bindings.glsl"

out vec4 vColor;

void main()
{
    gl_Position = uView.worldToClip * vec4(aPosition, 1.0);
	vColor = aColor;
}
