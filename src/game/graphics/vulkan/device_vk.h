#pragma once

#include "gfx_core/device.h"
#include "gfx_core/gpu.h"
#include "gfx_core/buffer.h"
#include "vma_allocator.h"

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

    std::vector<gpu> enumerate_gpus() const override;

    buffer create_buffer(u64 size, buffer_usage usage, memory_type mem_type, bool persistant) override;
    void free_buffer(buffer* buf) override;

    texture create_texture(texture_info info, resource_view_type view_type, memory_type mem_type, bool persistant) override;
    void free_texture(texture* tex) override;

    void map(buffer* buf) override;
    void map(texture* tex) override;
    void unmap(buffer* buf) override;
    void unmap(texture* tex) override;

    void wait_idle() override;

    VkQueue get_queue(u32 familyIdx, u32 idx = 0) const;
    VkQueue get_queue_by_flags(VkQueueFlags flags, u32 idx = 0) const;
    VkQueue get_queue_by_present(u32 idx = 0) const;
    u32 get_family_index_by_flags(VkQueueFlags flags) const;
    bool queue_family_supports_present(u32 familyIdx) const;

    void dump_info() const override;
private:
    void create_instance();

    std::vector<const char*> get_instance_extensions() const;
    std::vector<const char*> get_instance_layers() const;
    std::vector<const char*> get_device_extensions() const;

    void map_impl(memory_info* memInfo);
    void unmap_impl(memory_info* memInfo);
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

#ifdef GFX_VK_VMA_ALLOCATOR
    vma_allocator m_allocator;
#endif
};

} // gfx