#include "command_list.h"
#include "Driver.h"

namespace gfx
{

void command_list::init()
{
    m_pso.reset();
    m_operationFlags = 0;
    m_isActive = false;
}

void command_list::reset()
{
    GFX_ASSERT(!m_isActive, "Command list should not be reset whilst it is active.");
    m_pso.reset();
    m_operationFlags = 0;
    GFX_CALL(reset, this);
}

void command_list::begin()
{
    GFX_ASSERT(!m_isActive, "Command list cannot begin whilst it is already active.");
    GFX_CALL(begin, this);
    m_isActive = true;
}

void command_list::end()
{
    GFX_ASSERT(m_isActive, "Command list must be active in order for it to be ended.");
    GFX_CALL(end, this);
    m_isActive = false;
}

void command_list::submit()
{
    GFX_ASSERT(!m_isActive, "Command list cannot be submitted whilst it's still active.");
    std::vector<command_list*> lists
    {
        this
    };

    GFX_CALL(submit, lists);
}

const command_operation_flags& command_list::get_operation_flags() const
{
    return m_operationFlags;
}

void command_list::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
{
    m_operationFlags |= cmd_op_graphics;
    GFX_CALL(draw, this, vertex_count, instance_count, first_vertex, first_instance);
}

void command_list::draw_indexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance)
{
    m_operationFlags |= cmd_op_graphics;
    GFX_CALL(draw_indexed, this, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void command_list::bind_vertex_buffers(buffer* pBuffers, u32 buffer_count, u32 first_vertex_index)
{
    m_operationFlags |= cmd_op_graphics;
    GFX_CALL(bind_vertex_buffers, this, pBuffers, buffer_count, first_vertex_index);
}

void command_list::bind_index_buffer(buffer* buffer, index_buffer_type index_type)
{
    m_operationFlags |= cmd_op_graphics;
    GFX_CALL(bind_index_buffer, this, buffer, index_type);
}

} // gfx