#version 450

layout (location = 0) out vec4 v_color;

void main()
{
  //const array of positions for the triangle
  const vec3 positions[3] = vec3[3](
    vec3(-0.75, 0.75, 0.0),
    vec3(0.75, 0.75, 0.0),
    vec3(0.0,-0.75, 0.0)
  );
  
  const vec3 colors[3] = vec3[3](
    vec3(0.3, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.3)
  );

  //output the position of each vertex
  gl_Position = vec4(positions[gl_VertexIndex], 1.0);
    v_color = vec4(colors[gl_VertexIndex], 1.0);
}
