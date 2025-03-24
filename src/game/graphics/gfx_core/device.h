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
#include "descriptor_pool.h"

#include "swapchain.h"

#if GFX_VIRTUAL_DEVICE
    #define GFX_DEVICE_VIRTUAL_PREFIX virtual
    #define GFX_DEVICE_VIRTUAL_SUFFIX = 0
#else
    #define GFX_DEVICE_VIRTUAL_PREFIX
    #define GFX_DEVICE_VIRTUAL_SUFFIX
#endif

#define GFX_DEVICE_CALL(...) GFX_DEVICE_VIRTUAL_PREFIX __VA_ARGS__ GFX_DEVICE_VIRTUAL_SUFFIX

namespace gfx
{

class device
{
public:
    device() = default;
    GFX_DEVICE_VIRTUAL_PREFIX ~device() = default;

    DELETE_COPY(device);
    DELETE_MOVE(device);

    GFX_DEVICE_CALL(u32 initialise(u32 gpuIdx, surface_create_func surface_func));
    GFX_DEVICE_CALL(u32 initialise(u32 gpuIdx));
    GFX_DEVICE_CALL(void shutdown());

#ifdef GFX_EXT_SWAPCHAIN
    GFX_DEVICE_CALL(surface_capabilities get_surface_capabilities() const);

    GFX_DEVICE_CALL(swapchain create_swapchain(swapchain* previous, texture_info info, present_mode present_mode));
    GFX_DEVICE_CALL(void free_swapchain(swapchain* swapchain));

    GFX_DEVICE_CALL(u32 acquire_next_image(swapchain* sc, fence* fence, u64 timeout = u64_max));

    GFX_DEVICE_CALL(void present(swapchain* sc, u32 image_index, const std::vector<dependency*>& dependencies = { }));
#endif // GFX_EXT_SWAPCHAIN

    GFX_DEVICE_CALL(std::vector<gpu> enumerate_gpus() const);

    GFX_DEVICE_CALL(buffer create_buffer(u64 size, buffer_usage usage, memory_type mem_type, bool persistant));
    GFX_DEVICE_CALL(void free_buffer(buffer* buf));

    GFX_DEVICE_CALL(texture create_texture(texture_info info, resource_view_type view_type, memory_type mem_type, bool persistant));
    GFX_DEVICE_CALL(void free_texture(texture* tex));

    GFX_DEVICE_CALL(fence create_fence(bool signaled = false));
    GFX_DEVICE_CALL(void free_fence(fence* fence));

    GFX_DEVICE_CALL(dependency create_dependency(const char* debug_name = nullptr));
    GFX_DEVICE_CALL(void free_dependency(dependency* dep));

    GFX_DEVICE_CALL(graphics_command_list allocate_graphics_command_list());
    GFX_DEVICE_CALL(void free_command_list(command_list* list));

    GFX_DEVICE_CALL(void map(buffer* buf));
    GFX_DEVICE_CALL(void map(texture* tex));
    GFX_DEVICE_CALL(void unmap(texture* tex));
    GFX_DEVICE_CALL(void unmap(buffer* buf));

    GFX_DEVICE_CALL(void wait_idle());

    GFX_DEVICE_CALL(bool wait_for_fences(const fence* pFences, u32 count, bool wait_for_all, u64 timeout) const);
    GFX_DEVICE_CALL(bool reset_fences(fence* pFences, u32 count));
    GFX_DEVICE_CALL(bool check_fence(const fence* fence) const);

    // Command Lists
    GFX_DEVICE_CALL(void reset(command_list* list));
    GFX_DEVICE_CALL(void begin(command_list* list));
    GFX_DEVICE_CALL(void end(command_list* list));

    GFX_DEVICE_CALL(void submit(const std::vector<graphics_command_list*>& lists, fence* fence = nullptr));
    GFX_DEVICE_CALL(void submit(const std::vector<compute_command_list*>& lists, fence* fence = nullptr));
    GFX_DEVICE_CALL(void submit(const std::vector<present_command_list*>& lists, fence* fence = nullptr));

    // Commands
    GFX_DEVICE_CALL(void draw(command_list* list, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance));
    GFX_DEVICE_CALL(void draw_indexed(command_list* list, u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance));

    GFX_DEVICE_CALL(void bind_vertex_buffers(command_list* list, buffer** pBuffers, u32 buffer_count, u32 first_vertex_index));
    GFX_DEVICE_CALL(void bind_index_buffer(command_list* list, buffer* buffer, index_buffer_type type));

    GFX_DEVICE_CALL(void begin_rendering(command_list* list, texture** color_outputs, u32 color_output_count, texture* depth_output));
    GFX_DEVICE_CALL(void end_rendering(command_list* list));

    // TEMPORARY
    GFX_DEVICE_CALL(void begin_pass(command_list* list, program* program, u64 passIdx, texture* output));
    GFX_DEVICE_CALL(void end_pass(command_list* list));

    // GFX_DEVICE_CALL(void copy_texture(texture* src, texture_layout src_layout, texture* dst, texture_layout dst_layout, texture_region? region);
    GFX_DEVICE_CALL(void copy_texture_to_texture(command_list* list, texture* src, texture* dst));
    GFX_DEVICE_CALL(void copy_buffer_to_texture(command_list* list, buffer* src, texture* dst));
    GFX_DEVICE_CALL(void copy_buffer_to_buffer(command_list* list, buffer* src, buffer* dst));
    GFX_DEVICE_CALL(void texture_barrier(command_list* list, texture* texture, texture_layout dst_layout));

    // GFX_DEVICE_CALL(void set_vertex_input_state(vertex_input_state* pStates, u32 state_count = 1, u32 first_vertex_index), void* pAux = nullptr));

    // Shader things
    GFX_DEVICE_CALL(void* create_shader_pass_impl(program* program, u64 pass));
    GFX_DEVICE_CALL(void* create_shader_pass_layout_impl(pass* pass));

    GFX_DEVICE_CALL(void* create_descriptor_table_desc_impl(descriptor_table_desc* desc));
    GFX_DEVICE_CALL(void destroy_descriptor_table_desc(descriptor_table_desc* desc));

    GFX_DEVICE_CALL(void destroy_shader_program(program* program));

    GFX_DEVICE_CALL(void* create_descriptor_pool_impl(descriptor_table_desc* base, u32 size));
    GFX_DEVICE_CALL(void destroy_descriptor_pool(descriptor_pool* pool));
    GFX_DEVICE_CALL(void reset_descriptor_pool(descriptor_pool* pool));

    GFX_DEVICE_CALL(void* allocate_descriptor_table_impl(descriptor_pool* pool));
    GFX_DEVICE_CALL(void write_descriptor_table(descriptor_table* table));

    inline debugger& get_debugger()
    {
        return m_debugger;
    }

#ifdef GFX_SUPPORTS_VULKAN
    GFX_DEVICE_CALL(VkInstance get_vulkan_instance() const);
#endif

    GFX_DEVICE_CALL(void dump_info() const);
protected:
    template<typename T>
    T& get_impl()
    {
        return *static_cast<T*>(m_deviceImpl);
    }

    template<typename T>
    const T& get_impl() const
    {
        return *static_cast<T*>(m_deviceImpl);
    }
protected:
    debugger m_debugger{ };
    gpu m_gpu;
    void* m_surface{ };
    void* m_deviceImpl{ };
};

} // gfx