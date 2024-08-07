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

layout (location=0) in vec4 in_position;
layout (location=1) in vec4 in_normal;

layout (location=0) out vec3 out_frag_position;
layout (location=1) out vec3 out_normal;

void main()
{
  mat4 mvp = g_camera.proj * g_camera.view * g_camera.model;
  // out_frag_position = vec3(g_camera.model * in_position);
  
  out_normal = vec3(in_normal);
  gl_Position = mvp * in_position;
}