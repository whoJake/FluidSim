#pragma once

#include "gfxdefines.h"
#include "gpu.h"
#include "debugger.h"

#include "buffer.h"
#include "texture.h"
#include "fence.h"
#include "command_list.h"
#include "dependency.h"
#include "shader.h"

#include "swapchain.h"

namespace gfx
{

class device
{
public:
    device() = default;
    virtual ~device() = default;

    DELETE_COPY(device);
    DELETE_MOVE(device);

    virtual u32 initialise(u32 gpuIdx, void* surface) = 0;
    virtual u32 initialise(u32 gpuIdx) = 0;
    virtual void shutdown() = 0;

#ifdef GFX_EXT_SWAPCHAIN
    virtual surface_capabilities get_surface_capabilities() const = 0;

    virtual swapchain create_swapchain(swapchain* previous, texture_info info, present_mode present_mode) = 0;
    virtual void free_swapchain(swapchain* swapchain) = 0;

    virtual u32 acquire_next_image(swapchain* swapchain, fence* fence, u64 timeout = u64_max) = 0;

    virtual void present(swapchain* swapchain, u32 image_index, const std::vector<dependency*>& dependencies = { }) = 0;
#endif // GFX_EXT_SWAPCHAIN

    virtual std::vector<gpu> enumerate_gpus() const = 0;

    virtual buffer create_buffer(u64 size, buffer_usage usage, memory_type mem_type, bool persistant) = 0;
    virtual void free_buffer(buffer* buf) = 0;

    virtual texture create_texture(texture_info info, resource_view_type view_type, memory_type mem_type, bool persistant) = 0;
    virtual void free_texture(texture* tex) = 0;

    virtual fence create_fence(bool signaled = false) = 0;
    virtual void free_fence(fence* fence) = 0;

    virtual dependency create_dependency(const char* debug_name = nullptr) = 0;
    virtual void free_dependency(dependency* dep) = 0;

    virtual graphics_command_list allocate_graphics_command_list() = 0;
    virtual void free_command_list(command_list* list) = 0;

    virtual void map(buffer* buf) = 0;
    virtual void map(texture* tex) = 0;
    virtual void unmap(texture* tex) = 0;
    virtual void unmap(buffer* buf) = 0;

    virtual void wait_idle() = 0;

    virtual bool wait_for_fences(const fence* pFences, u32 count, bool wait_for_all, u64 timeout) const = 0;
    virtual bool reset_fences(fence* pFences, u32 count) = 0;
    virtual bool check_fence(const fence* fence) const = 0;

    // Command Lists
    virtual void reset(command_list* list) = 0;
    virtual void begin(command_list* list) = 0;
    virtual void end(command_list* list) = 0;

    virtual void submit(const std::vector<graphics_command_list*>& lists, fence* fence = nullptr) = 0;
    virtual void submit(const std::vector<compute_command_list*>& lists, fence* fence = nullptr) = 0;
    virtual void submit(const std::vector<present_command_list*>& lists, fence* fence = nullptr) = 0;

    // Commands
    virtual void draw(command_list* list, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) = 0;
    virtual void draw_indexed(command_list* list, u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) = 0;

    virtual void bind_vertex_buffers(command_list* list, buffer* pBuffers, u32 buffer_count, u32 first_vertex_index) = 0;
    virtual void bind_index_buffer(command_list* list, buffer* buffer, index_buffer_type type) = 0;

    // virtual void copy_texture(texture* src, texture_layout src_layout, texture* dst, texture_layout dst_layout, texture_region? region);
    virtual void copy_texture_to_texture(command_list* list, texture* src, texture* dst) = 0;
    virtual void copy_buffer_to_texture(command_list* list, buffer* src, texture* dst) = 0;
    virtual void texture_barrier(command_list* list, texture* texture, texture_layout dst_layout) = 0;

    // virtual void set_vertex_input_state(vertex_input_state* pStates, u32 state_count = 1, u32 first_vertex_index = 0, void* pAux = nullptr) = 0;

    // Shader things
    virtual void* create_shader_pass_impl(program* program, u64 pass) = 0;
    virtual void* create_shader_pass_layout_impl(pass* pass) = 0;

    virtual void* create_descriptor_table_desc_impl(descriptor_table_desc* desc) = 0;
    virtual void destroy_descriptor_table_desc(descriptor_table_desc* desc) = 0;

    virtual void destroy_shader_program(program* program) = 0;

    virtual void write_descriptor_table(descriptor_table* table) = 0;

    inline debugger& get_debugger()
    {
        return m_debugger;
    }

    virtual void dump_info() const = 0;
protected:
    debugger m_debugger{ };
    void* m_surface{ };
};

} // gfx