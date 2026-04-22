#version 450 core

#extension GL_NV_gpu_shader5 : enable

in vec4 vColor;

layout(location = 0) out vec3 oColor;

void main()
{
	oColor = vColor.rgb;
}
