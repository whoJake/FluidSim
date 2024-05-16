#include "PhysicalDevice.h"
#include "Instance.h"

#include <algorithm>

namespace vk
{

PhysicalDevice::PhysicalDevice(const Instance& instance, VkPhysicalDevice physicalDevice) :
    m_instance(instance),
    m_handle(physicalDevice),
    m_requestedFeatures()
{
    vkGetPhysicalDeviceFeatures(m_handle, &m_features);
    vkGetPhysicalDeviceProperties(m_handle, &m_properties);
    vkGetPhysicalDeviceMemoryProperties(m_handle, &m_memoryProperties);

    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, nullptr);

    m_queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &queueFamilyCount, m_queueFamilyProperties.data());

    JCLOG_INFO(m_instance.get_log(), "\t{}", std::string(m_properties.deviceName).c_str());
}

const VkPhysicalDeviceFeatures& PhysicalDevice::get_features() const
{
    return m_features;
}

const VkPhysicalDeviceProperties& PhysicalDevice::get_properties() const
{
    return m_properties;
}

const VkPhysicalDeviceMemoryProperties& PhysicalDevice::get_memory_properties() const
{
    return m_memoryProperties;
}

const std::vector<VkQueueFamilyProperties>& PhysicalDevice::get_queue_family_properties() const
{
    return m_queueFamilyProperties;
}

VkPhysicalDevice PhysicalDevice::get_handle() const
{
    return m_handle;
}

const Instance& PhysicalDevice::get_instance() const
{
    return m_instance;
}

bool PhysicalDevice::is_present_supported(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const
{
    VkBool32 support{ VK_FALSE };
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(m_handle, queueFamilyIndex, surface, &support);
    VK_CHECK(result, "Failed to get Physical Device surface support state.");
    return support;
}

void PhysicalDevice::request_features(VkPhysicalDeviceFeatures features)
{
    JCLOG_DEBUG(m_instance.get_log(), "Physical Device features requested: ");
    auto requested = get_device_features_overlap(features, features, true);
    for( auto& req : requested )
    {
        JCLOG_DEBUG(m_instance.get_log(), "\t{}", req.c_str());
    }

    auto unsupported = get_device_features_overlap(features, m_features, false);
    if( unsupported.size() > 0 )
    {
        JCLOG_ERROR(m_instance.get_log(), "The following features were requested but aren't available on this physical device:");
        for( std::string& feature : unsupported )
        {
            JCLOG_ERROR(m_instance.get_log(), "\t{}", feature.c_str());
        }
        QUITFMT("Features were requested but not available on the physical device selected. See above for details.");
    }
    m_requestedFeatures = operator_bitor(m_requestedFeatures, features);
}

const VkPhysicalDeviceFeatures& PhysicalDevice::get_requested_features() const
{
    return m_requestedFeatures;
}

uint32_t PhysicalDevice::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memoryProperties{ };
    vkGetPhysicalDeviceMemoryProperties(m_handle, &memoryProperties);

    for( uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++ )
    {
        if( typeFilter & (1 << i) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties )
        {
            return i;
        }
    }
    QUITFMT("Failed to find memory type that matches the filter requested.");
    return 0;
}

} // vk