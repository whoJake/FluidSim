#pragma once

#include "resource.h"
#include "memory.h"
#include "types.h"
#include "gfxdefines.h"

namespace gfx
{

class buffer : public resource
{
public:
    buffer() = default;
    ~buffer() = default;

    DEFAULT_MOVE(buffer);
    DEFAULT_COPY(buffer);

    void initialise(buffer&& other);
    void initialise(memory_info allocation, buffer_usage usage, void* pImpl);

    buffer_usage get_usage() const;

    GFX_HAS_IMPL(m_pImpl);
private:
    void* m_pImpl;
    buffer_usage m_usage;
protected:
    u32 m_extFlags;
};

class index_buffer : public buffer
{
public:
    index_buffer() = default;
    ~index_buffer() = default;

    DEFAULT_MOVE(index_buffer);
    DEFAULT_COPY(index_buffer);

    void initialise(buffer&& buffer, index_buffer_type type);

    index_buffer_type get_index_buffer_type() const;
    void set_index_buffer_type(index_buffer_type type);
};

class vertex_buffer_input_state
{
public:
    vertex_buffer_input_state() :
        m_channels(),
        m_channelCount(0)
    { }
    vertex_buffer_input_state(vertex_type* pVertexTypes, format* pFormats, u32 count);
    ~vertex_buffer_input_state() = default;

    DEFAULT_MOVE(vertex_buffer_input_state);
    DEFAULT_COPY(vertex_buffer_input_state);

    vertex_type get_index_type(u32 index) const;
    void set_index_type(u32 index, vertex_type type);

    format get_index_format(u32 index) const;
    void set_index_format(u32 index, format format);

    u32 get_channel_count() const;
    void set_channel_count(u32 count);
private:
    static constexpr u32 max_packed_vertex_channels = 6;
    struct
    {
        u16 type : 3;
        u16 format : 8;
        u16 unused : 5;
    }m_channels[max_packed_vertex_channels];

    u16 m_channelCount;
};

class vertex_buffer : public buffer
{
public:
    vertex_buffer() = default;
    ~vertex_buffer() = default;

    DEFAULT_MOVE(vertex_buffer);
    DEFAULT_COPY(vertex_buffer);

    void initialise(buffer&& buffer, vertex_buffer_input_state input_state = { });

    u32 get_vertex_stride() const;

    const vertex_buffer_input_state& get_input_state() const;
    vertex_buffer_input_state& get_input_state();
private:
    vertex_buffer_input_state m_inputState;
};

} // gfx