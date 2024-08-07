#version 450

layout (set=0, binding=0) uniform settings
{
	vec2 offset;
	vec2 scale;
	vec3 color;
}ub_settings;

layout (location=0) out vec4 out_color;

void main()
{
  out_color = vec4(ub_settings.color, 1.0);
}