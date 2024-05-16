#pragma once

#include "vkcommon.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "ResourceCache.h"

namespace vk
{

/**
* @brief Wrapper class for VkDevice
*
* Responsible for creating the Vulkan Device with the extensions requested and
* holding onto key information surrounding it.
*/
class Device
{
public:
    /**
    * @brief Constructor for Device
    * @param "gpu" A valid physical device with its requested gpu features
    * @param "surface" Platform specific surface
    * @param "requestedExtensions" (Optional) List of extensions that should be enabled on the VkDevice
    */
    Device(PhysicalDevice&                 gpu,
           VkSurfaceKHR                    surface,
           const std::vector<const char*>& requestedExtensions = { });
    ~Device();

    Device(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(const Device&) = delete;
    Device& operator=(Device&&) = delete;

    VkDevice get_handle() const;
    const jclog::Log& get_log() const;
    const PhysicalDevice& get_gpu() const;
    VkSurfaceKHR get_surface() const;
    const Queue& get_queue(uint32_t familyIndex, uint32_t index) const;
    const Queue& get_queue_by_flags(VkQueueFlags flags, uint32_t index) const;
    const Queue& get_queue_by_present(uint32_t index) const;

    bool is_extension_supported(std::string extension) const;

    bool is_extension_enabled(std::string extension) const;

    VkResult wait_idle() const;

    VmaAllocator get_allocator() const;

    ResourceCache& get_resource_cache();

private:
    VkDevice m_handle{ VK_NULL_HANDLE };
    jclog::Log& m_log;

    const PhysicalDevice& m_gpu;

    VkSurfaceKHR m_surface;
    std::vector<VkExtensionProperties> m_deviceExtensions;
    std::vector<const char*> m_enabledExtensions;

    std::vector<std::vector<Queue>> m_queues;

    VmaAllocator m_allocator;

    ResourceCache m_resourceCache;
};

} // vk