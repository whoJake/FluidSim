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

layout (location=0) in vec3 in_normal;
layout (location=1) in vec3 in_position;

layout (location=0) out vec4 out_color;

void main()
{
	vec3 pointlight_pos = vec3(0.0, 1.0, 0.0) * 15.0;
	vec3 pointlight_dir = normalize(pointlight_pos - in_position);
	float pointlight_strength = 20.0;
	
	float strength = 1.0 - clamp(length(in_position - pointlight_pos) / pointlight_strength, 0.0, 1.0);
	float diffuse = max(dot(pointlight_dir, in_normal), 0.0) * strength;
	out_color = vec4(diffuse, diffuse, diffuse, 1.0);
}