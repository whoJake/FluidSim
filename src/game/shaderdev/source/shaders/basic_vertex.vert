#version 450

layout ( push_constant ) uniform constants
{
mat4 proj;
mat4 view;
mat4 model;
}
pc_matrices;

layout(location=0) in
vec3 in_position;
layout (location=1) in
vec3 in_normal;
layout (location=2) in
vec3 in_colour;

layout (location=0) out
vec3 out_normal;
layout (location=1) out
vec4 out_color;
layout (location=2) out
vec3 out_frag_position;

void main()
{
    mat4 mvp = pc_matrices.proj * pc_matrices.view * pc_matrices.model;
    gl_Position = mvp * vec4(in_position, 1.0);
    out_normal = in_normal;
    out_frag_position = vec3(pc_matrices.model * vec4(in_position, 1.0));
    out_color = vec4(in_colour, 1.0);
}