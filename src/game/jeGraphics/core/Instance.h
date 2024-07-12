#pragma once

#include "vkcommon.h"

PARAM(vulkan_debug_utils);

namespace vk
{

class PhysicalDevice;

/**
* @brief Wrapper class for VkInstance
*
* Responsible for initializing Vulkan, checking and enabling all provided extensions
* and validation layers as well as used for acquiring available physical devices.
*/
class Instance
{
public:

    /**
    * @brief Initializes the instance used to interact with Vulkan
    * @param "log"                      Log object to use for this instance.
    * @param "applicationName"          Name of the application
    * @param "engineName"               Name of the engine
    * @param "apiVersion"               Version of the Vulkan API that the instance will be created with
    * @param "enabledExtensions"        Name of all extensions that should be enabled
    * @param "enabledValidationLayers"  Name of all validation layers that should be enabled
    *
    * @throws runtime_error if enabled extensions or validation names are not found
    */
    Instance(const std::string&             applicationName,
             const std::string&             engineName,
             uint32_t                       apiVersion,
             const std::vector<const char*> enabledExtensions       = { },
             const std::vector<const char*> enabledValidationLayers = { });

    ~Instance();

    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&&) = delete;

    /**
    * @brief Queries all visible physical devices on the machine
    */
    void query_gpus();

    /**
    * @brief Finds the first detected descrete GPU
    * @returns A valid physical device
    */
    PhysicalDevice& get_first_gpu() const;

    VkInstance get_handle() const;

    uint32_t get_api_version() const;

    const std::vector<const char*>& get_enabled_extensions() const;

    /**
    * @brief Checks if the given extension is enabled on this instance
    * @param "extension" Name of extension to check
    */
    bool is_extension_enabled(const char* extension) const;

private:
    /* @brief The Vulkan instance handle */
    VkInstance m_handle{ VK_NULL_HANDLE };

    uint32_t m_apiVersion;

    /* @brief The enabled extensions */
    std::vector<const char*> m_enabledExtensions;

    /* @brief Debug utils messenger used by VK_EXT_Debug_Utils */
    VkDebugUtilsMessengerEXT m_debugUtilsMessenger{ VK_NULL_HANDLE };

    std::vector<std::unique_ptr<PhysicalDevice>> m_gpus;
};

} // vk