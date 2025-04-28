#include "context.h"

namespace gfx
{

bool context::is_active() const
{
    // TODO this probably should change.
    return m_cmdList;
}

u32 context::get_active_frame_index() const
{
    return m_activeFrameIdx;
}

u32 context::get_id() const
{
    return m_contextId;
}

void context::set_id(u32 id)
{
    m_contextId = id;
}

void context::copy_texture(texture* src, texture* dst)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->copy_to_texture(src, dst);
}

void context::copy_buffer(buffer* src, texture* dst)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->copy_to_texture(src, dst);
}

void context::copy_buffer(buffer* src, buffer* dst)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->copy_buffer(src, dst);
}

void context::texture_layout_transition(texture* target, texture_layout dst_layout, pipeline_stage_flag_bits src_stage, pipeline_stage_flag_bits dst_stage)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->texture_memory_barrier(target, dst_layout, src_stage, dst_stage);
}

void context::bind_program(program* program, u64 passIdx)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->bind_program(program, passIdx);
}

void context::bind_descriptor_table(pass* pass, descriptor_table* table, descriptor_table_type type)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->bind_descriptor_tables(pass, &table, 1, type);
}

void graphics_context::begin_rendering(const std::vector<texture_attachment>& color_attachments, texture_attachment* depth_attachment)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    get_command_list()->begin_rendering(color_attachments, depth_attachment);
}

void graphics_context::end_rendering()
{
    GFX_ASSERT(is_active(), "Context is not active.");
    get_command_list()->end_rendering();
}

void graphics_context::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->draw(vertex_count, instance_count, first_vertex, first_instance);
}

void graphics_context::draw_indexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);
}

void graphics_context::bind_vertex_buffers(const std::vector<buffer*>& buffers, u32 first_vertex_index)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->bind_vertex_buffers(const_cast<buffer**>(buffers.data()), u32_cast(buffers.size()), first_vertex_index);
}

void graphics_context::bind_index_buffer(buffer* pBuffer, index_buffer_type index_type)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->bind_index_buffer(pBuffer, index_type);
}

void graphics_context::set_viewport(f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->set_viewport(x, y, width, height, min_depth, max_depth);
}

void graphics_context::set_scissor(u32 x, u32 y, u32 width, u32 height)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->set_scissor(x, y, width, height);
}

void graphics_context::begin(graphics_command_list* list)
{
    GFX_ASSERT(!is_active(), "Context is already active.");
    m_cmdList = list;
    m_cmdList->reset(false);
    m_cmdList->begin();
}

graphics_command_list* graphics_context::end()
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->end();
    graphics_command_list* retval = get_command_list();
    m_cmdList = nullptr;
    return retval;
}

graphics_command_list* graphics_context::get_command_list() const
{
    return static_cast<graphics_command_list*>(m_cmdList);
}

} // gfx