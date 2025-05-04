#pragma once

#include "../device.h"
#include "../gpu.h"
#include "../buffer.h"
#include "../command_list.h"

#include "vma_allocator.h"
#include "command_pool_vk.h"

#if GFX_VIRTUAL_DEVICE
    #define VK_DEVICE device_vk
#else
    #define VK_DEVICE device
#endif

namespace gfx
{

enum queue_family_flag_bit : u32
{
    supports_present = 1 << 0,
    max = u32_max,
};
using queue_family_flags = u32;

struct queue_family
{
    std::vector<VkQueue> queues;
    VkQueueFamilyProperties properties;
    u32 index;
    queue_family_flags flags;
    u64 unused;
};

#if GFX_VIRTUAL_DEVICE
class device_vk : public device
{
public:
    u32 initialise(u32 gpuIdx, surface_create_func surface_func) override;
    u32 initialise(u32 gpuIdx) override;
    void shutdown() override;

#ifdef GFX_EXT_SWAPCHAIN
    surface_capabilities get_surface_capabilities() const override;

    swapchain create_swapchain(swapchain* previous, texture_info info, u32 image_count, texture_usage_flags usage, format format, present_mode present_mode) override;
    void free_swapchain(swapchain* swapchain) override;

    swapchain_acquire_result acquire_next_image(swapchain* swapchain, u32* aquired_index, dependency* signal_dep, fence* signal_fence, u64 timeout = u64_max) override;

    void present(swapchain* swapchain, u32 image_index, const std::vector<dependency*>& dependencies) override;
#endif // GFX_EXT_SWAPCHAIN

    screen_capabilities query_screen_capabilities() const override;

    std::vector<gpu> enumerate_gpus() const override;

    void* allocate_buffer(resource* resource, const memory_info& memory_info) override;
    void free_buffer(buffer* buf) override;

    void* create_buffer_view_impl(buffer_view* view, buffer_view_range range) override;
    void destroy_buffer_view_impl(buffer_view* view) override;

    void* allocate_texture(texture* texture, const memory_info& memory_info, resource_view_type view_type) override;
    void free_texture(texture* tex) override;

    void* create_texture_view_impl(texture_view* view, texture_view_range range) override;
    void destroy_texture_view_impl(texture_view* view) override;

    void* create_texture_sampler_impl(texture_sampler* sampler) override;
    void destroy_texture_sampler_impl(texture_sampler* sampler) override;

    u8* map_resource(const resource* resouce) override;
    void unmap_resource(const resource* resource) override;

    fence create_fence(bool signaled = false) override;
    void free_fence(fence* fence) override;

    dependency create_dependency(const char* debug_name) override;
    void free_dependency(dependency* dep) override;

    graphics_command_list allocate_graphics_command_list(bool secondary = false) override;
    void free_command_list(command_list* list) override;

    void wait_idle() override;

    bool wait_for_fences(const fence* pFences, u32 count, bool wait_for_all, u64 timeout) const override;
    bool reset_fences(fence* pFences, u32 count) override;
    bool check_fence(const fence* fence) const override;

    void reset(command_list* list) override;
    void begin(command_list* list) override;
    void end(command_list* list) override;

    // TODO this needs fences, semaphores, dependencies.. etc
    // should this be stored on the command_list, "add dependency", "execute after"
    // or passed in here.
    // Same with pipeline stages (on the Vulkan side atleast). How do I handle this.
    // The fence should probably be per batch submit, but these lists can be split up over multiple
    // vkQueueSubmit, depending on what queue family they go to.
    void submit(const std::vector<graphics_command_list*>& lists, fence* fence) override;
    void submit(const std::vector<compute_command_list*>& lists, fence* fence) override;
    void submit(const std::vector<present_command_list*>& lists, fence* fence) override;

    // Graphics calls
    void draw(command_list* list, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) override;
    void draw_indexed(command_list* list, u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance) override;

    void bind_program(command_list* list, program* prog, u64 passIdx) override;
    void bind_vertex_buffers(command_list* list, buffer** pBuffers, u32 buffer_count, u32 first_vertex_index) override;
    void bind_index_buffer(command_list* list, buffer* buffer, index_buffer_type type) override;

    void bind_descriptor_tables(command_list* list, pass* pass, descriptor_table** pTables, u32 table_count, descriptor_table_type type) override;

    void begin_rendering(command_list* list, const std::vector<texture_attachment>& color_outputs, texture_attachment* depth_output) override;
    void end_rendering(command_list* list) override;

    void set_viewport(command_list* list, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth) override;
    void set_scissor(command_list* list, u32 x, u32 y, u32 width, u32 height) override;

    void copy_texture_to_texture(command_list* list, texture* src, texture* dst) override;
    void copy_buffer_to_texture(command_list* list, buffer* src, texture* dst) override;
    void copy_buffer_to_buffer(command_list* list, buffer* src, buffer* dst) override;
    void texture_barrier(command_list* list, texture* texture, texture_layout dst_layout, pipeline_stage_flag_bits src_stage, pipeline_stage_flag_bits dst_stage) override;

    void execute_command_lists(command_list* list, command_list** execute_lists, u32 count) override;

    // Shader things
    void* create_shader_pass_impl(program* program, u64 passIdx) override;
    void* create_shader_pass_layout_impl(pass* pass) override;

    void* create_descriptor_table_desc_impl(descriptor_table_desc* desc) override;
    void destroy_descriptor_table_desc(descriptor_table_desc* desc) override;

    void destroy_shader_program(program* program) override;

    void* create_descriptor_pool_impl(descriptor_table_desc* base, u32 size, bool reuse_tables) override;
    void destroy_descriptor_pool(descriptor_pool* pool) override;
    void reset_descriptor_pool(descriptor_pool* pool) override;

    void* allocate_descriptor_table_impl(descriptor_pool* pool) override;
    void write_descriptor_table(descriptor_table* table) override;

    void dump_info() const override;
};
#endif // GFX_VIRTUAL_DEVICE

struct device_state_vk
{
    VkInstance instance;

    std::vector<std::string> available_instance_extensions;
    std::vector<const char*> enabled_instance_extensions;

    std::vector<std::string> available_instance_layers;
    std::vector<const char*> enabled_instance_layers;

    VkDevice device;

    std::vector<std::string> available_device_extensions;
    std::vector<const char*> enabled_device_extensions;

    std::vector<queue_family> queue_families;

    command_pool_vk command_pool;
#ifdef GFX_VK_VMA_ALLOCATOR
    vma_allocator allocator;
#endif

    void create_instance(debugger& debugger);

    VkQueue get_queue(u32 familyIdx, u32 idx = 0) const;
    VkQueue get_queue_by_flags(VkQueueFlags flags, u32 idx = 0) const;
    VkQueue get_queue_by_present(u32 idx = 0) const;
    u32 get_queue_family_count() const;
    u32 get_family_index_by_flags(VkQueueFlags flags) const;
    bool queue_family_supports_present(u32 familyIdx) const;

    // bool has_instance_extension(const char* name);
    // bool has_device_extension(const char* name);
};

constexpr std::vector<const char*> vulkan_get_instance_extensions()
{
    return { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_surface", "VK_KHR_win32_surface" };
}

constexpr std::vector<const char*> vulkan_get_instance_layers()
{
    return { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };
}

constexpr std::vector<const char*> vulkan_get_device_extensions()
{
    return { "VK_KHR_swapchain", "VK_KHR_dynamic_rendering" };
}

} // gfx