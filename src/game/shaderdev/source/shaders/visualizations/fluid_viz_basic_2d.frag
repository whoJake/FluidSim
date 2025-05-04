#version 450

layout (location = 0) in vec3 fin_color;
layout (location = 1) in vec3 fin_velocity;
layout (location = 2) in vec2 fin_localNodePosition;
layout (location = 3) in vec2 fin_localFragPosition;
layout (location = 4) in float fin_localRadius;

layout (location = 0) out vec4 out_fragColor;

void main()
{
  if(abs(distance(fin_localNodePosition, fin_localFragPosition)) > fin_localRadius)
    discard;
  else
    out_fragColor = vec4(fin_color, 1.0);
}
