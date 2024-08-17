#version 450

layout (set=0, binding=0) uniform frameData
{
	float deltaTime;
	uint frame;
	uint swapChainIndex;
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

layout (location=0) out vec4 out_color;

void main()
{
	vec3 sun = normalize(vec3(0.2, 0.3, 0.7));
	float diffuse = dot(sun, in_normal);
	diffuse = clamp(diffuse, 0.05, 1.0);

	out_color = vec4(diffuse, diffuse, diffuse, 1.0);
}