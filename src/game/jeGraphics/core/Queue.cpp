#include "Queue.h"
#include "Device.h"

namespace vk
{

Queue::Queue(Device&                 device,
             uint32_t                queueFamilyIndex,
             VkQueueFamilyProperties properties,
             bool                    presentSupport,
             uint32_t                queueIndex) :
    m_device(device),
    m_handle(VK_NULL_HANDLE),
    m_familyIndex(queueFamilyIndex),
    m_index(queueIndex),
    m_presentSupport(presentSupport),
    m_properties(properties)
{
    vkGetDeviceQueue(m_device.get_handle(), m_familyIndex, m_index, &m_handle);
}

Queue::Queue(Queue&& other) :
    m_device(other.m_device),
    m_handle(other.m_handle),
    m_familyIndex(other.m_familyIndex),
    m_index(other.m_index),
    m_presentSupport(other.m_presentSupport),
    m_properties(other.m_properties)
{
    other.m_handle = VK_NULL_HANDLE;
    other.m_familyIndex = { };
    other.m_index = { };
    other.m_presentSupport = VK_FALSE;
    other.m_properties = { };
}

const Device& Queue::get_device() const
{
    return m_device;
}

VkQueue Queue::get_handle() const
{
    return m_handle;
}

uint32_t Queue::get_family_index() const
{
    return m_familyIndex;
}

uint32_t Queue::get_index() const
{
    return m_index;
}

const VkQueueFamilyProperties& Queue::get_properties() const
{
    return m_properties;
}

bool Queue::has_present_support() const
{
    return m_presentSupport;
}

VkResult Queue::submit(const std::vector<VkSubmitInfo>& submitInfos, VkFence fence) const
{
    return vkQueueSubmit(m_handle, to_u32(submitInfos.size()), submitInfos.data(), fence);
}

VkResult Queue::present(const VkPresentInfoKHR& presentInfo) const
{
    if( !m_presentSupport )
        return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;

    return vkQueuePresentKHR(m_handle, &presentInfo);
}

VkResult Queue::wait_idle() const
{
    return vkQueueWaitIdle(m_handle);
}

} // vk