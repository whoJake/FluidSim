#pragma once

#include "gfxdefines.h"
#include "gpu.h"
#include "debugger.h"

#include "resource_view.h"
#include "buffer.h"
#include "texture.h"
#include "texture_sampler.h"
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

#define GFX_DEVICE_FUNC(...) GFX_DEVICE_VIRTUAL_PREFIX __VA_ARGS__ GFX_DEVICE_VIRTUAL_SUFFIX

namespace gfx
{

struct screen_capabilities;

class device
{
public:
    device() = default;
    GFX_DEVICE_VIRTUAL_PREFIX ~device() = default;

    DELETE_COPY(device);
    DELETE_MOVE(device);

    GFX_DEVICE_FUNC(u32 initialise(u32 gpuIdx, surface_create_func surface_func));
    GFX_DEVICE_FUNC(u32 initialise(u32 gpuIdx));
    GFX_DEVICE_FUNC(void shutdown());

#ifdef GFX_EXT_SWAPCHAIN
    GFX_DEVICE_FUNC(surface_capabilities get_surface_capabilities() const);

    GFX_DEVICE_FUNC(swapchain create_swapchain(swapchain* previous, texture_info info, u32 image_count, texture_usage_flags usage, format format, present_mode present_mode));
    GFX_DEVICE_FUNC(void free_swapchain(swapchain* swapchain));

    GFX_DEVICE_FUNC(swapchain_acquire_result acquire_next_image(swapchain* sc, u32* aquired_index, dependency* signal_dep, fence* signal_fence, u64 timeout = u64_max));

    GFX_DEVICE_FUNC(void present(swapchain* sc, u32 image_index, const std::vector<dependency*>& dependencies = { }));
#endif // GFX_EXT_SWAPCHAIN

    GFX_DEVICE_FUNC(screen_capabilities query_screen_capabilities() const);

    GFX_DEVICE_FUNC(std::vector<gpu> enumerate_gpus() const);

    GFX_DEVICE_FUNC(void* allocate_buffer(resource* resource, const memory_info& memory_info));
    GFX_DEVICE_FUNC(void free_buffer(buffer* buffer));

    GFX_DEVICE_FUNC(void* create_buffer_view_impl(buffer_view* view, buffer_view_range range));
    GFX_DEVICE_FUNC(void destroy_buffer_view_impl(buffer_view* view));

    GFX_DEVICE_FUNC(void* allocate_texture(texture* texture, const memory_info& memory_info, resource_view_type view_type));
    GFX_DEVICE_FUNC(void free_texture(texture* texture));

    GFX_DEVICE_FUNC(void* create_texture_view_impl(texture_view* view, texture_view_range range));
    GFX_DEVICE_FUNC(void destroy_texture_view_impl(texture_view* view));

    GFX_DEVICE_FUNC(void* create_texture_sampler_impl(texture_sampler* sampler));
    GFX_DEVICE_FUNC(void destroy_texture_sampler_impl(texture_sampler* sampler));

    GFX_DEVICE_FUNC(u8* map_resource(const resource* resource));
    GFX_DEVICE_FUNC(void unmap_resource(const resource* resource));

    GFX_DEVICE_FUNC(fence create_fence(bool signaled = false));
    GFX_DEVICE_FUNC(void free_fence(fence* fence));

    GFX_DEVICE_FUNC(dependency create_dependency(const char* debug_name = nullptr));
    GFX_DEVICE_FUNC(void free_dependency(dependency* dep));

    GFX_DEVICE_FUNC(graphics_command_list allocate_graphics_command_list(bool secondary = false));
    GFX_DEVICE_FUNC(void free_command_list(command_list* list));

    GFX_DEVICE_FUNC(void wait_idle());

    GFX_DEVICE_FUNC(bool wait_for_fences(const fence* pFences, u32 count, bool wait_for_all, u64 timeout) const);
    GFX_DEVICE_FUNC(bool reset_fences(fence* pFences, u32 count));
    GFX_DEVICE_FUNC(bool check_fence(const fence* fence) const);

    // Command Lists
    GFX_DEVICE_FUNC(void reset(command_list* list));
    GFX_DEVICE_FUNC(void begin(command_list* list));
    GFX_DEVICE_FUNC(void end(command_list* list));

    GFX_DEVICE_FUNC(void submit(const std::vector<graphics_command_list*>& lists, fence* fence = nullptr));
    GFX_DEVICE_FUNC(void submit(const std::vector<compute_command_list*>& lists, fence* fence = nullptr));
    GFX_DEVICE_FUNC(void submit(const std::vector<present_command_list*>& lists, fence* fence = nullptr));

    // Commands
    GFX_DEVICE_FUNC(void draw(command_list* list, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance));
    GFX_DEVICE_FUNC(void draw_indexed(command_list* list, u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance));

    GFX_DEVICE_FUNC(void bind_program(command_list* list, program* prog, u64 passIdx));
    GFX_DEVICE_FUNC(void bind_vertex_buffers(command_list* list, buffer** pBuffers, u32 buffer_count, u32 first_vertex_index));
    GFX_DEVICE_FUNC(void bind_index_buffer(command_list* list, buffer* buffer, index_buffer_type type));

    GFX_DEVICE_FUNC(void bind_descriptor_tables(command_list* list, pass* pass, descriptor_table** pTables, u32 table_count, descriptor_table_type type));

    GFX_DEVICE_FUNC(void begin_rendering(command_list* list, const std::vector<texture_attachment>& color_outputs, texture_attachment* depth_output));
    GFX_DEVICE_FUNC(void end_rendering(command_list* list));

    GFX_DEVICE_FUNC(void set_viewport(command_list* list, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth));
    GFX_DEVICE_FUNC(void set_scissor(command_list* list, u32 x, u32 y, u32 width, u32 height));

    // GFX_DEVICE_FUNC(void copy_texture(texture* src, texture_layout src_layout, texture* dst, texture_layout dst_layout, texture_region? region);
    GFX_DEVICE_FUNC(void copy_texture_to_texture(command_list* list, texture* src, texture* dst));
    GFX_DEVICE_FUNC(void copy_buffer_to_texture(command_list* list, buffer* src, texture* dst));
    GFX_DEVICE_FUNC(void copy_buffer_to_buffer(command_list* list, buffer* src, buffer* dst));
    GFX_DEVICE_FUNC(void texture_barrier(command_list* list, texture* texture, texture_layout dst_layout, pipeline_stage_flag_bits src_stage = PIPELINE_STAGE_BOTTOM_OF_PIPE, pipeline_stage_flag_bits dst_stage = PIPELINE_STAGE_TOP_OF_PIPE));

    GFX_DEVICE_FUNC(void execute_command_lists(command_list* list, command_list** execute_lists, u32 count));

    // GFX_DEVICE_FUNC(void set_vertex_input_state(vertex_input_state* pStates, u32 state_count = 1, u32 first_vertex_index), void* pAux = nullptr));

    // Shader things
    GFX_DEVICE_FUNC(void* create_shader_pass_impl(program* program, u64 pass));
    GFX_DEVICE_FUNC(void* create_shader_pass_layout_impl(pass* pass));

    GFX_DEVICE_FUNC(void* create_descriptor_table_desc_impl(descriptor_table_desc* desc));
    GFX_DEVICE_FUNC(void destroy_descriptor_table_desc(descriptor_table_desc* desc));

    GFX_DEVICE_FUNC(void destroy_shader_program(program* program));

    GFX_DEVICE_FUNC(void* create_descriptor_pool_impl(descriptor_table_desc* base, u32 size, bool reuse_tables));
    GFX_DEVICE_FUNC(void destroy_descriptor_pool(descriptor_pool* pool));
    GFX_DEVICE_FUNC(void reset_descriptor_pool(descriptor_pool* pool));

    GFX_DEVICE_FUNC(void* allocate_descriptor_table_impl(descriptor_pool* pool));
    GFX_DEVICE_FUNC(void write_descriptor_table(descriptor_table* table));

    GFX_DEVICE_FUNC(void dump_info() const);

    inline const gpu& get_gpu() const
    {
        return m_gpu;
    }

    inline debugger& get_debugger()
    {
        return m_debugger;
    }

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