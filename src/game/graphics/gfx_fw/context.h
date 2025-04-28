#pragma once
#include "gfx_core/texture.h"
#include "gfx_core/buffer.h"
#include "gfx_core/command_list.h"

namespace gfx
{

/// <summary>
/// Context : Inherently single threaded. Each thread will have its own context
/// for both graphics and compute.
/// </summary>
class context
{
public:
    bool is_active() const;
    u32 get_active_frame_index() const;

    void set_id(u32 id);
    u32 get_id() const;

    // Commands
    void copy_texture(texture* src, texture* dst);

    void copy_buffer(buffer* src, texture* dst);
    void copy_buffer(buffer* src, buffer* dst);

    void texture_layout_transition(texture* target, texture_layout dst_layout, pipeline_stage_flag_bits src_stage = PIPELINE_STAGE_BOTTOM_OF_PIPE, pipeline_stage_flag_bits dst_stage = PIPELINE_STAGE_TOP_OF_PIPE);

    void bind_program(program* program, u64 passIdx);
    void bind_descriptor_table(pass* pass, descriptor_table* table, descriptor_table_type type);
protected:
    context() = default;
    ~context() = default;
protected:
    transfer_command_list* m_cmdList;
private:
    u32 m_contextId{ u32_max };
    u32 m_activeFrameIdx;
};

class graphics_context : public context
{
public:
    graphics_context() = default;
    ~graphics_context() = default;

    void begin_rendering(const std::vector<texture_attachment>& color_attachments, texture_attachment* depth_attachment);
    void end_rendering();

    void draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0);
    void draw_indexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0, u32 vertex_offset = 0, u32 first_instance = 0);

    void bind_vertex_buffers(const std::vector<buffer*>& buffers, u32 first_vertex_index = 0);
    void bind_index_buffer(buffer* buffer, index_buffer_type index_type = INDEX_TYPE_U16);

    void set_viewport(f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth);
    void set_scissor(u32 x, u32 y, u32 width, u32 height);

    void begin(graphics_command_list* list);
    graphics_command_list* end();

    graphics_command_list* get_command_list() const;
};

} // gfx