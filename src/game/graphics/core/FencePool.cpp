#include "FencePool.h"
#include "Device.h"

namespace vk
{

FencePool::FencePool(Device& device) :
    m_device(device)
{ }

FencePool::~FencePool()
{
    wait();
    reset();

    for( VkFence fence : m_fences )
    {
        vkDestroyFence(m_device.get_handle(), fence, nullptr);
    }
}

VkFence FencePool::request_fence()
{
    if( m_activeFences < m_fences.size() )
    {
        return m_fences[m_activeFences++];
    }

    VkFence createdFence{ VK_NULL_HANDLE };
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    createInfo.flags = 0;

    VkResult fenceResult = vkCreateFence(m_device.get_handle(), &createInfo, nullptr, &createdFence);
    VK_CHECK(fenceResult, "Failed to create new fence in the FencePool.");

    m_fences.push_back(createdFence);
    m_activeFences++;
    return createdFence;
}

VkResult FencePool::wait(uint64_t timeout) const
{
    if( m_activeFences == 0 )
        return VK_SUCCESS;

    return vkWaitForFences(m_device.get_handle(), m_activeFences, m_fences.data(), VK_TRUE, timeout);
}

VkResult FencePool::reset()
{
    if( m_activeFences == 0 || m_fences.empty() )
        return VK_SUCCESS;

    VkResult resetResult = vkResetFences(m_device.get_handle(), m_activeFences, m_fences.data());
    VK_CHECK(resetResult, "Failed to reset fences in FencePool. There are {} active fences trying to be reset.", m_activeFences);

    m_activeFences = 0;
    return VK_SUCCESS;
}

} // vk