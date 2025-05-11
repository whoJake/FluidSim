#version 450

//output write
layout (location = 0) out vec4 outFragColor;
layout (location = 0) in vec4 v_color;

void main()
{
	outFragColor = v_color;
}
