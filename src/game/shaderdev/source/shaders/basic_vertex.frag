#version 450

layout (location=0) in vec3 in_normal;
layout (location=1) in vec4 in_color;
layout (location=2) in vec3 in_frag_position;

layout (location=0) out vec4 out_color;

void main()
{
  vec3 lightPosition = vec3(100.0, 100.0, 100.0);
  vec3 lightDirection = normalize(lightPosition - in_frag_position);
  float diffuse = clamp(max(dot(in_normal, lightDirection), 0.0), 0.4, 1.0);

  out_color = in_color * diffuse;
}