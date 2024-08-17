#version 450

layout (set=0, binding=0) uniform frameData
{
	float deltaTime;
	uint frame;
	uint swapChainIndex;
	uint unused;
}g_frameData;

layout (set=1, binding=0) uniform camera
{
	mat4 proj;
	mat4 view;
	
	mat4 proj_view;
	
	vec3 worldPosition;
	vec2 clipDistances;
	
	float unused[3];
}g_camera;

layout (set=2, binding=0) uniform model
{
	mat4 model;
}u_model;

layout (location=0) in vec4 in_position;
layout (location=1) in vec4 in_normal;
// layout (location=2) in vec2 in_texcoords0;


layout (location=0) out vec3 out_normal;

void main()
{
	out_normal = in_normal.xyz;
	mat4 mvp = g_camera.proj_view * u_model.model;
	gl_Position = mvp * in_position;
}