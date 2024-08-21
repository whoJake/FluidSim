#pragma once

struct RenderFrameData
{
    float delta_time;
    u32 frame_idx;
    u32 swapchain_idx;
};

struct RenderCameraData
{
    glm::mat4 projection_matrix;
    glm::mat4 view_matrix;

    glm::mat4 projection_view_matrix;

    glm::vec3 world_position;
    glm::vec2 clip_distances;

    u32 unused[3];
};