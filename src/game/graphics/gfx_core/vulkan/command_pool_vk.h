#pragma once
#include "vkdefines.h"
#include <mutex>

namespace gfx
{

class device_vk;
struct device_state_vk;

class command_pool_vk
{
public:
    command_pool_vk() = default;
    ~command_pool_vk() = default;

    void initialise(device_state_vk* state);
    void shutdown();

    VkCommandBuffer allocate_buffer(u32 family_index);
    VkCommandBuffer allocate_buffer_by_flags(VkQueueFlags flags);

    void free_buffer(VkCommandBuffer buffer, u32 family_index);
    void free_buffer_by_flags(VkCommandBuffer buffer, VkQueueFlags flags);
private:
    device_state_vk* m_state{ nullptr };
    VkCommandPool* m_pools{ nullptr };
    u64* m_buffersActive{ nullptr };

    std::mutex* m_locks{ nullptr };

    // Not really needed, we could do device_vk::get_queue_family_count
    // but incase something happens its best to keep this local.
    u64 m_poolCount{ 0 };
};

} // gfx