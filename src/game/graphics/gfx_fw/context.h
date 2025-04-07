#pragma once
#include "gfx_core/texture.h"
#include "gfx_core/buffer.h"
#include "gfx_core/command_list.h"

namespace gfx
{

class context
{
public:
    bool is_active() const;
    u32 get_active_frame_index() const;

    // Commands
    void copy_texture(texture* src, texture* dst);
    void copy_texture(buffer* src, texture* dst);
    void copy_buffer(buffer* src, buffer* dst);
protected:
    context() = default;
    ~context() = default;
protected:
    transfer_command_list* m_cmdList;

private:
    u32 m_activeFrameIdx;
};

class graphics_context : public context
{
public:
    graphics_context() = default;
    ~graphics_context() = default;

    void draw(u32 vertex_count, u32 instance_count = 1, u32 first_vertex = 0, u32 first_instance = 0);

    void bind_vertex_buffers(buffer** pBuffers, u32 buffer_count, u32 first_vertex_index = 0);
private:
    graphics_command_list* get_command_list() const;
};

} // gfx