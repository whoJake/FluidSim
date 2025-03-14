#include "command_list.h"
#include "Driver.h"

namespace gfx
{

command_list::command_list(command_list_type type) :
    m_type(type)
{ }

void command_list::init(void* pImpl)
{
    m_pso.reset();
    m_isActive = false;
    m_pImpl = pImpl;
}

void command_list::reset(bool keep_dependencies)
{
    GFX_ASSERT(!m_isActive, "Command list should not be reset whilst it is active.");
    m_pso.reset();
    GFX_CALL(reset, this);

    if( !keep_dependencies )
    {
        m_waitDependencies.clear();
        m_signalDependency = nullptr;
    }
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

void command_list::submit(fence* fence)
{
    GFX_ASSERT(!m_isActive, "Command list cannot be submitted whilst it's still active.");

    switch( m_type )
    {
    case command_list_type::graphics:
    {
        std::vector<graphics_command_list*> list
        {
            reinterpret_cast<graphics_command_list*>(this)
        };
        GFX_CALL(submit, list, fence);
        break;
    }
    case command_list_type::compute:
    {
        std::vector<compute_command_list*> list
        {
            reinterpret_cast<compute_command_list*>(this)
        };
        GFX_CALL(submit, list, fence);
        break;
    }
    case command_list_type::present:
    {
        std::vector<present_command_list*> list
        {
            reinterpret_cast<present_command_list*>(this)
        };
        GFX_CALL(submit, list, fence);
        break;
    }
    default:
        GFX_ASSERT(false, "Submit has been called with an invalid/uninitialised command_list.");
    }
}

void command_list::texture_memory_barrier(texture* texture, texture_layout dst_layout)
{
    GFX_CALL(texture_barrier, this, texture, dst_layout);
    texture->set_layout(dst_layout);
}

const command_list_type& command_list::get_type() const
{
    return m_type;
}

void command_list::add_wait_dependency(dependency* dep)
{
    if( std::find(m_waitDependencies.begin(), m_waitDependencies.end(), dep) != m_waitDependencies.end() )
    {
        m_waitDependencies.push_back(dep);
    }
}

void command_list::remove_wait_dependency(dependency* dep)
{
    auto it = std::find(m_waitDependencies.begin(), m_waitDependencies.end(), dep);
    if( it != m_waitDependencies.end() )
    {
        m_waitDependencies.erase(it);
    }
}

const std::vector<dependency*>& command_list::get_wait_dependencies() const
{
    return m_waitDependencies;
}

void command_list::set_signal_dependency(dependency* dep)
{
    m_signalDependency = dep;
}

const dependency* command_list::get_signal_dependency() const
{
    return m_signalDependency;
}

transfer_command_list::transfer_command_list() :
    command_list(command_list_type::transfer)
{ }

transfer_command_list::transfer_command_list(command_list_type override_type) :
    command_list(override_type)
{ }

void transfer_command_list::copy_to_texture(texture* src, texture* dst)
{
    GFX_CALL(copy_texture_to_texture, reinterpret_cast<command_list*>(this), src, dst);
}

void transfer_command_list::copy_to_texture(buffer* src, texture* dst)
{
    GFX_CALL(copy_buffer_to_texture, reinterpret_cast<command_list*>(this), src, dst);
}

void transfer_command_list::copy_buffer(buffer* src, buffer* dst)
{
    GFX_CALL(copy_buffer_to_buffer, static_cast<command_list*>(this), src, dst);
}

graphics_command_list::graphics_command_list() :
    transfer_command_list(command_list_type::graphics)
{ }

void graphics_command_list::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
{
    GFX_CALL(draw, reinterpret_cast<command_list*>(this), vertex_count, instance_count, first_vertex, first_instance);
}

void graphics_command_list::draw_indexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance)
{
    GFX_CALL(draw_indexed, reinterpret_cast<command_list*>(this), index_count, instance_count, first_index, vertex_offset, first_instance);
}

void graphics_command_list::bind_vertex_buffers(const std::vector<buffer*>& buffers, u32 first_vertex_index)
{
    GFX_CALL(bind_vertex_buffers, reinterpret_cast<command_list*>(this), buffers, first_vertex_index);
}

void graphics_command_list::bind_index_buffer(buffer* buffer, index_buffer_type index_type)
{
    GFX_CALL(bind_index_buffer, reinterpret_cast<command_list*>(this), buffer, index_type);
}

compute_command_list::compute_command_list() :
    transfer_command_list(command_list_type::compute)
{ }

present_command_list::present_command_list() :
    command_list(command_list_type::present)
{ }

} // gfx