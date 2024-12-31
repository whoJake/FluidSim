#pragma once

#include "pipeline_state.h"
#include "buffer.h"
#include "fence.h"

namespace gfx
{

enum class command_list_type
{
    graphics = 0,
    compute,
    transfer,
    present,

    count,
};

class command_list
{
public:
    ~command_list() = default;

    void init(void* pImpl);
    void reset();

    void begin();
    void end();

    void submit(fence* fence = nullptr);
    const command_list_type& get_type() const;
    
    // commands
    GFX_HAS_IMPL(m_pImpl);
protected:
    command_list(command_list_type type);

    pipeline_state m_pso;
    command_list_type m_type;
    bool m_isActive{ false };

private:
    u8 m_unused[3];
    void* m_pImpl;
};

class transfer_command_list : public command_list
{
public:
    transfer_command_list();
    ~transfer_command_list() = default;


    // Commands

protected:
    transfer_command_list(command_list_type override_type);
};

class graphics_command_list : public transfer_command_list
{
public:
    graphics_command_list();
    ~graphics_command_list() = default;
    
    // Commands
    void draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0);
    void draw_indexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0, u32 vertex_offset = 0, u32 first_instance = 0);

    void bind_vertex_buffers(buffer* pBuffers, u32 buffer_count = 1, u32 first_vertex_index = 0);
    void bind_index_buffer(buffer* buffer, index_buffer_type index_type = index_buffer_type::u16_type);
};

class compute_command_list : public transfer_command_list
{
public:
    compute_command_list();
    ~compute_command_list() = default;

    // Commands
};

class present_command_list : public command_list
{
public:
    present_command_list();
    ~present_command_list() = default;

    // Commands
};

} // gfx