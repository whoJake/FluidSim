#version 450

layout (location = 0) in vec3 fin_color;
layout (location = 1) in vec3 fin_velocity;
layout (location = 2) in vec2 fin_clipNodePosition;
layout (location = 3) in vec2 fin_clipFragPosition;
layout (location = 4) in float fin_clipRadius;

layout (location = 0) out vec4 out_fragColor;

void main()
{
  if(abs(distance(fin_clipNodePosition, fin_clipFragPosition)) > fin_clipRadius)
    discard;
  else
    // out_fragColor = vec4(fin_velocity.xy, 1.0, 1.0);
    out_fragColor = vec4(fin_color, 1.0);
}
