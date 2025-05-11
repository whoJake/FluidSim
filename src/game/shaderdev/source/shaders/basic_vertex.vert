#version 450

layout (set = 0, binding = 0) uniform PerObjectMatrices
{
  mat4 proj;
  mat4 view;
  mat4 model;
}perObj;

layout (location=0) in
vec3 in_position;
layout (location=1) in
vec3 in_normal;

layout (location=0) out
vec3 out_normal;
layout (location=1) out
vec4 out_color;
layout (location=2) out
vec3 out_frag_position;

void main()
{
    mat4 mvp = perObj.proj * perObj.view * perObj.model;
    gl_Position = mvp * vec4(in_position, 1.0);
    out_normal = in_normal;
    out_frag_position = vec3(perObj.model * vec4(in_position, 1.0));
    out_color = vec4(0.0, 1.0, 0.0, 1.0);
}