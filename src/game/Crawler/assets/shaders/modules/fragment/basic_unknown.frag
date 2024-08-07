#version 450

layout (set=0, binding=0) uniform camera
{
	mat4 proj;
	mat4 view;
	
	// temp
	mat4 model;
	mat4 mvp;
}g_camera;

/*
layout (set=1, binding=0) uniform model
{
	mat4 model;
	mat4 mvp;
}u_model;
*/

layout (location=0) in vec3 in_frag_position;
layout (location=1) in vec3 in_normal;
layout (location=0) out vec4 out_color;

void main()
{
	out_color = vec4(in_normal, 1.0);
}