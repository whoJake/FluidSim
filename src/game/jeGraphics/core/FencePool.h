#pragma once
#include "vkcommon.h"

namespace vk
{

class Device;

class FencePool
{
public:
    FencePool(Device& device);
    ~FencePool();

    FencePool(const FencePool&) = delete;
    FencePool(FencePool&&) = delete;
    FencePool& operator=(const FencePool&) = delete;
    FencePool& operator=(FencePool&&) = delete;

    VkFence request_fence();

    VkResult wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;

    VkResult reset();
private:
    Device& m_device;

    std::vector<VkFence> m_fences;

    uint32_t m_activeFences{ 0 };
};

} // vk