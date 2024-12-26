#pragma once

#include "pipeline_state.h"
#include "buffer.h"

namespace gfx
{

enum command_operation_bits : u32
{
    cmd_op_graphics = 1 << 0,
    cmd_op_compute = 1 << 1,
    cmd_op_present = 1 << 2,

    cmd_op_count,
};
using command_operation_flags = std::underlying_type_t<command_operation_bits>;

class command_list
{
public:
    command_list() = default;
    ~command_list() = default;

    void init();
    void reset();

    void begin();
    void end();

    void submit();
    const command_operation_flags& get_operation_flags() const;
    
    // commands
    void draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0);
    void draw_indexed(u32 index_count, u32 instance_count = 1, u32 first_index = 0, u32 vertex_offset = 0, u32 first_instance = 0);

    void bind_vertex_buffers(buffer* pBuffers, u32 buffer_count = 1, u32 first_vertex_index = 0);
    void bind_index_buffer(buffer* buffer, index_buffer_type index_type = index_buffer_type::u16_type);

    GFX_HAS_IMPL(pImpl);
private:
    pipeline_state m_pso;
    void* pImpl;
    command_operation_flags m_operationFlags;
    bool m_isActive{ false };
};

} // gfx