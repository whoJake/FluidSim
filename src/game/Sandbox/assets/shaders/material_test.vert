#version 450

layout (set=0, binding=0) uniform settings
{
	vec2 offset;
	vec2 scale;
	vec3 color;
}ub_settings;

vec2 positions[3] = vec2[]
(
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

void main()
{
  gl_Position = vec4((positions[gl_VertexIndex] * ub_settings.scale) + ub_settings.offset, 0.5, 1.0);
  // gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}