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
	vec3 sun = normalize(vec3(0.2, 0.3, -0.4));
	float diffuse = dot(sun, in_normal);
	diffuse = clamp(diffuse, 0.05, 1.0);

	out_color = vec4(diffuse, diffuse, diffuse, 1.0);
}