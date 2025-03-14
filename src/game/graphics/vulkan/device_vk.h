#pragma once

#include "gfx_core/device.h"
#include "gfx_core/gpu.h"
#include "gfx_core/buffer.h"
#include "gfx_core/command_list.h"

#include "vma_allocator.h"
#include "command_pool_vk.h"

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

class device_vk : public device
{
public:
    device_vk();
    ~device_vk() override;

    u32 initialise(u32 gpuIdx, void* surface) override;
    u32 initialise(u32 gpuIdx) override;
    void shutdown() override;

#ifdef GFX_EXT_SWAPCHAIN
    surface_capabilities get_surface_capabilities() const override;

    swapchain create_swapchain(swapchain* previous, texture_info info, present_mode present_mode) override;
    void free_swapchain(swapchain* swapchain) override;

    u32 acquire_next_image(swapchain* swapchain, fence* fence, u64 timeout = u64_max) override;

    void present(swapchain* swapchain, u32 image_index, const std::vector<dependency*>& dependencies) override;
#endif // GFX_EXT_SWAPCHAIN

    std::vector<gpu> enumerate_gpus() const override;

    buffer create_buffer(u64 size, buffer_usage usage, memory_type mem_type, bool persistant) override;
    void free_buffer(buffer* buf) override;

    texture create_texture(texture_info info, resource_view_type view_type, memory_type mem_type, bool persistant) override;
    void free_texture(texture* tex) override;

    fence create_fence(bool signaled = false) override;
    void free_fence(fence* fence) override;

    dependency create_dependency(const char* debug_name) override;
    void free_dependency(dependency* dep) override;

    graphics_command_list allocate_graphics_command_list() override;
    void free_command_list(command_list* list) override;

    void map(buffer* buf) override;
    void map(texture* tex) override;
    void unmap(buffer* buf) override;
    void unmap(texture* tex) override;

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

    void bind_vertex_buffers(command_list* list, const std::vector<buffer*>& buffers, u32 first_vertex_index) override;
    void bind_index_buffer(command_list* list, buffer* buffer, index_buffer_type type) override;

    void begin_pass(command_list* list, program* program, u64 passIdx, texture* output) override;
    void end_pass(command_list* list) override;

    void copy_texture_to_texture(command_list* list, texture* src, texture* dst) override;
    void copy_buffer_to_texture(command_list* list, buffer* src, texture* dst) override;
    void copy_buffer_to_buffer(command_list* list, buffer* src, buffer* dst) override;
    void texture_barrier(command_list* list, texture* texture, texture_layout dst_layout) override;

    // Shader things
    void* create_shader_pass_impl(program* program, u64 passIdx) override;
    void* create_shader_pass_layout_impl(pass* pass) override;

    void* create_descriptor_table_desc_impl(descriptor_table_desc* desc) override;
    void destroy_descriptor_table_desc(descriptor_table_desc* desc) override;

    void destroy_shader_program(program* program) override;

    void* create_descriptor_pool_impl(descriptor_table_desc* base, u32 size) override;
    void destroy_descriptor_pool(descriptor_pool* pool) override;
    void reset_descriptor_pool(descriptor_pool* pool) override;

    void* allocate_descriptor_table_impl(descriptor_pool* pool) override;
    void write_descriptor_table(descriptor_table* table) override;

    // Internal VK functions
    VkQueue get_queue(u32 familyIdx, u32 idx = 0) const;
    VkQueue get_queue_by_flags(VkQueueFlags flags, u32 idx = 0) const;
    VkQueue get_queue_by_present(u32 idx = 0) const;
    u32 get_queue_family_count() const;
    u32 get_family_index_by_flags(VkQueueFlags flags) const;
    bool queue_family_supports_present(u32 familyIdx) const;

    bool has_instance_extension(const char* name);
    bool has_device_extension(const char* name);

    VkInstance get_impl_instance() const;
    VkDevice get_impl_device() const;

    void dump_info() const override;
private:
    void create_instance();

    std::vector<const char*> get_instance_extensions() const;
    std::vector<const char*> get_instance_layers() const;
    std::vector<const char*> get_device_extensions() const;

    void map_impl(memory_info* memInfo);
    void unmap_impl(memory_info* memInfo);

    // TODO fence?
    void submit_impl(VkQueue queue, const std::vector<command_list*>& lists, fence* fence);

    VkPipeline create_graphics_pipeline_impl(program* program, u64 passIdx);
    VkPipeline create_compute_pipeline_impl(program* program, u64 passIdx);
private:
    VkInstance m_instance;

    std::vector<std::string> m_availableInstanceExtensions;
    std::vector<const char*> m_enabledInstanceExtensions;

    std::vector<std::string> m_availableInstanceLayers;
    std::vector<const char*> m_enabledInstanceLayers;
    
    gpu m_gpu;
    VkDevice m_device;

    std::vector<std::string> m_availableDeviceExtensions;
    std::vector<const char*> m_enabledDeviceExtensions;

    std::vector<queue_family> m_queueFamilies;

    command_pool_vk m_commandPool;
#ifdef GFX_VK_VMA_ALLOCATOR
    vma_allocator m_allocator;
#endif
};

} // gfx