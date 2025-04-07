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

void context::copy_texture(texture* src, texture* dst)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->copy_to_texture(src, dst);
}

void context::copy_texture(buffer* src, texture* dst)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->copy_to_texture(src, dst);
}

void context::copy_buffer(buffer* src, buffer* dst)
{
    GFX_ASSERT(is_active(), "Context is not active.");
    m_cmdList->copy_buffer(src, dst);
}

void graphics_context::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->draw(vertex_count, instance_count, first_vertex, first_instance);
}

void graphics_context::bind_vertex_buffers(buffer** pBuffers, u32 buffer_count, u32 first_vertex_index)
{
    GFX_ASSERT(is_active(), "Graphics context is not active.");
    get_command_list()->bind_vertex_buffers(pBuffers, buffer_count, first_vertex_index);
}

graphics_command_list* graphics_context::get_command_list() const
{
    return static_cast<graphics_command_list*>(m_cmdList);
}

} // gfx