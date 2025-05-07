#version 450

struct FluidNode
{
  vec2 position;
  vec2 velocity;
  float radius;
  
  float pad0;
  float pad1;
  float pad2;
};

layout (std140, set = 0, binding = 0) readonly buffer Viewport
{
	vec2 screen_extent;
	vec2 view_position;
	vec2 view_extent;
	vec2 padding;
} g_viewport;

layout (std140, set = 0, binding = 1) readonly buffer FluidNodeList
{
  FluidNode nodes[];
}in_nodeList;

layout (location = 0) out vec3 fin_color;
layout (location = 1) out vec3 fin_velocity;
layout (location = 2) out vec2 fin_clipNodePosition;
layout (location = 3) out vec2 fin_clipFragPosition;
layout (location = 4) out float fin_clipRadius;

//const array of positions for the triangle
const vec2 square_positions[4] = vec2[4](
  vec2(-0.5, -0.5), // TL 0
  vec2(-0.5, 0.5),  // TR 1
  vec2(0.5, -0.5),  // BL 2
  vec2(0.5, 0.5)    // BR 3
);

const int square_indices[6] = int[6](
  2, 1, 0,
  1, 2, 3
);

const float node_size = 2.0;

vec2 transform_to_screenspace(vec2 position)
{
	vec2 translated = position + g_viewport.view_position;
	// vec2 scaled = translated * (g_viewport.screen_extent / g_viewport.view_extent);
	// vec2 screenspace = scaled / g_viewport.screen_extent;
	return translated / g_viewport.view_extent;
}

vec2 transform_to_clip(vec2 position)
{
    vec2 local = transform_to_screenspace(position);
	vec2 clipspace = (local * 2.0) - vec2(1.0, 1.0);
	return clipspace * vec2(1.0, -1.0);
}

void main()
{
  vec2 node_position = in_nodeList.nodes[gl_InstanceIndex].position;
  float node_radius = in_nodeList.nodes[gl_InstanceIndex].radius;

  fin_clipNodePosition = transform_to_clip(node_position);
  fin_clipRadius = node_radius / g_viewport.view_extent.x;

  vec2 vertex_pos = node_position + (square_positions[square_indices[gl_VertexIndex]] * node_radius * 2);
  gl_Position = vec4(transform_to_clip(vertex_pos), 0.0, 1.0);
  fin_clipFragPosition = gl_Position.xy;
  
  fin_color = vec3(1.0, 1.0, 1.0);
  fin_velocity = vec3(in_nodeList.nodes[gl_InstanceIndex].velocity, 0.0);
}