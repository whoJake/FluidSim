#pragma once
#include "vkcommon.h"

namespace vk
{

class Device;

class SemaphorePool
{
public:
    SemaphorePool(Device& device);
    ~SemaphorePool();

    SemaphorePool(const SemaphorePool&) = delete;
    SemaphorePool(SemaphorePool&&) = delete;
    SemaphorePool& operator=(const SemaphorePool&) = delete;
    SemaphorePool& operator=(SemaphorePool&&) = delete;

    void reset();

    VkSemaphore request_semaphore();

    VkSemaphore request_semaphore_with_ownership();

    void release_owned_semaphore(VkSemaphore semaphore);

    uint32_t get_active_semaphore_count() const;
private:
    Device& m_device;

    std::vector<VkSemaphore> m_sempahores;
    std::vector<VkSemaphore> m_releasedSemaphores;

    uint32_t m_activeSemaphores{ 0 };
};

} // vk