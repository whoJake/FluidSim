#include "buffer.h"

namespace gfx
{

void buffer::initialise(buffer&& other)
{
    *this = std::move(other);
}

void buffer::initialise(memory_info allocation, buffer_usage usage, void* pImpl)
{
    resource::initialise(allocation);
    m_pImpl = pImpl;
    m_usage = usage;
    m_extFlags = 0;
}

buffer_usage buffer::get_usage() const
{
    return m_usage;
}

void index_buffer::initialise(buffer&& buf, index_buffer_type type)
{
    buffer::initialise(std::move(buf));
    set_index_buffer_type(type);
}

index_buffer_type index_buffer::get_index_buffer_type() const
{
    return static_cast<index_buffer_type>(m_extFlags);
}

void index_buffer::set_index_buffer_type(index_buffer_type type)
{
    m_extFlags = u32_cast(type);
}

vertex_buffer_input_state::vertex_buffer_input_state(vertex_type* pVertexTypes, format* pFormats, u32 count) :
    m_channels{ },
    m_channelCount(u16_cast(count))
{
    for( u32 i = 0; i < count; i++ )
    {
        set_index_type(i, pVertexTypes[i]);
        set_index_format(i, pFormats[i]);
    }
}

vertex_type vertex_buffer_input_state::get_index_type(u32 index) const
{
    return static_cast<vertex_type>(m_channels[index].type);
}

void vertex_buffer_input_state::set_index_type(u32 index, vertex_type type)
{
    m_channels[index].type = u16_cast(type);
}

format vertex_buffer_input_state::get_index_format(u32 index) const
{
    return static_cast<format>(m_channels[index].format);
}

void vertex_buffer_input_state::set_index_format(u32 index, format format)
{
    m_channels[index].format = u16_cast(format);
}

u32 vertex_buffer_input_state::get_channel_count() const
{
    return u32_cast(m_channelCount);
}

void vertex_buffer_input_state::set_channel_count(u32 count)
{
    m_channelCount = u16_cast(count);
}

void vertex_buffer::initialise(buffer&& buf, vertex_buffer_input_state input_state)
{
    buffer::initialise(std::move(buf));
    m_inputState = input_state;
}

u32 vertex_buffer::get_vertex_stride() const
{
    u32 retval = 0;
    for( u32 i = 0; i < m_inputState.get_channel_count(); i++ )
    {
        retval += get_format_stride(m_inputState.get_index_format(i));
    }
    return retval;
}

const vertex_buffer_input_state& vertex_buffer::get_input_state() const
{
    return m_inputState;
}

vertex_buffer_input_state& vertex_buffer::get_input_state()
{
    return m_inputState;
}

} // gfx