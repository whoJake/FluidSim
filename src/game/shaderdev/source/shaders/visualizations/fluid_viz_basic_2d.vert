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

layout (std140, set = 0, binding = 0) readonly buffer FluidNodeList
{
  FluidNode nodes[];
}in_nodeList;

layout (std140, set = 0, binding = 1) readonly buffer FrameInfo
{
  vec2 sim_to_local_ratio;
}in_frameInfo;

layout (location = 0) out vec3 fin_color;
layout (location = 1) out vec3 fin_velocity;
layout (location = 2) out vec2 fin_localNodePosition;
layout (location = 3) out vec2 fin_localFragPosition;
layout (location = 4) out float fin_localRadius;

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

void main()
{
  vec2 node_position = in_nodeList.nodes[gl_InstanceIndex].position;
  vec2 inter_node_position = node_position * in_frameInfo.sim_to_local_ratio;
  fin_localNodePosition = ((inter_node_position * 2.0) - vec2(1.0, 1.0)) * vec2(1.0, -1.0); // times at the end to flip
  
  fin_localRadius = in_nodeList.nodes[gl_InstanceIndex].radius * in_frameInfo.sim_to_local_ratio.x;
  vec2 square_vertex = square_positions[square_indices[gl_VertexIndex]];
  vec2 scaled_square_vertex = square_vertex * fin_localRadius * 2;
  fin_localFragPosition = fin_localNodePosition + scaled_square_vertex;
  gl_Position = vec4(fin_localFragPosition, 0.0, 1.0);
  
  fin_color = vec3(1.0, 1.0, 1.0);
  fin_velocity = vec3(in_nodeList.nodes[gl_InstanceIndex].velocity, 0.0);
}