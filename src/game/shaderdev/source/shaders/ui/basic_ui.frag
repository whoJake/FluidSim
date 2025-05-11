#version 450

layout (location=0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;
layout (set=0, binding=0) uniform sampler2D in_sampler0;

void main()
{
  out_color = texture(in_sampler0, in_uv);
}
