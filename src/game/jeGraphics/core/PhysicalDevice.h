#pragma once

#include "vkcommon.h"

namespace vk
{

class Instance;

/**
* @brief Wrapper for VkPhysicalDevice
*
* Responsible for holding onto physical device features, properties, memory properties
* and queue family properties for needed for Device creation
*/
class PhysicalDevice
{
public:
    PhysicalDevice(const Instance& instance, VkPhysicalDevice physicalDevice);

    PhysicalDevice(const PhysicalDevice&) = delete;
    PhysicalDevice(PhysicalDevice&&) = delete;
    PhysicalDevice& operator=(const PhysicalDevice&) = delete;
    PhysicalDevice& operator=(PhysicalDevice&&) = delete;

    const VkPhysicalDeviceFeatures& get_features() const;
    const VkPhysicalDeviceProperties& get_properties() const;
    const VkPhysicalDeviceMemoryProperties& get_memory_properties() const;
    const std::vector<VkQueueFamilyProperties>& get_queue_family_properties() const;

    VkPhysicalDevice get_handle() const;
    const Instance& get_instance() const;

    bool is_present_supported(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const;

    void request_features(VkPhysicalDeviceFeatures features);

    const VkPhysicalDeviceFeatures& get_requested_features() const;

    uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

private:
    const Instance& m_instance;

    VkPhysicalDevice m_handle{ VK_NULL_HANDLE };

    VkPhysicalDeviceFeatures m_features;
    VkPhysicalDeviceProperties m_properties;
    VkPhysicalDeviceMemoryProperties m_memoryProperties;

    std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
    VkPhysicalDeviceFeatures m_requestedFeatures{ };
};

} // vk
