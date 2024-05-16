#pragma once

#include "vkcommon.h"

namespace vk
{

class Device;

/**
* @brief Wrapper class for VkQueue
*
* Given a device and a queue family index, will get the queue from the device and
* hold onto relevent information
*/
class Queue
{
public:
    Queue(Device&                 device,
          uint32_t                queueFamilyIndex,
          VkQueueFamilyProperties properties,
          bool                    presentSupport,
          uint32_t                queueIndex        = 0);

    Queue(const Queue& other) = delete;
    Queue(Queue&& other);
    Queue& operator=(const Queue& other) = delete;
    Queue& operator=(Queue&& other) = delete;

    const Device& get_device() const;
    VkQueue get_handle() const;

    uint32_t get_family_index() const;
    uint32_t get_index() const;
    const VkQueueFamilyProperties& get_properties() const;

    bool has_present_support() const;

    VkResult submit(const std::vector<VkSubmitInfo>& submitInfos, VkFence fence) const;
    VkResult present(const VkPresentInfoKHR& presentInfo) const;

    VkResult wait_idle() const;

private:
    Device& m_device;
    VkQueue m_handle;

    uint32_t m_familyIndex;
    uint32_t m_index;

    bool m_presentSupport;

    VkQueueFamilyProperties m_properties;
};

} // vk