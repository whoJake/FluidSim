#define VMA_IMPLEMENTATION // Single vma include?
#include "Instance.h"

#include "PhysicalDevice.h"
#include <algorithm>

namespace vk
{

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    if( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
        VULKAN_ERROR(pCallbackData->pMessage);
    else if( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
        VULKAN_WARN(pCallbackData->pMessage);
    else
        VULKAN_INFO(pCallbackData->pMessage);

    return VK_FALSE;
}

static VkResult create_debug_utils_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if( func != nullptr )
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void destroy_debug_utils_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if( func != nullptr )
        func(instance, debugMessenger, pAllocator);
}

Instance::Instance(const std::string&             applicationName,
                   const std::string&             engineName,
                   uint32_t                       apiVersion,
                   const std::vector<const char*> enabledExtensions,
                   const std::vector<const char*> enabledValidationLayers) :
    m_apiVersion(apiVersion),
    m_enabledExtensions(enabledExtensions)
{
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    if( !enabledValidationLayers.empty() )
    {
        uint32_t validationLayerCount{ 0 };
        vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(validationLayerCount);
        vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

        /// log available layers
        VULKAN_DEBUG("Available Instance Layers: ");
        for( auto& layer : availableLayers )
            VULKAN_DEBUG("\t{}", std::string(layer.layerName).c_str());

        for( const char* layerName : enabledValidationLayers )
        {
            bool foundLayer{ false };

            for( const auto& layerProperty : availableLayers )
            {
                if( strcmp(layerName, layerProperty.layerName) == 0 ) {
                    foundLayer = true;
                    break;
                }
            }

            if( !foundLayer )
            {
                VULKAN_ERROR("Validation layer {} requested but not available", layerName);
                QUITFMT("Requested validation layer which is not available. See above for details.");
            }
        }

        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        //  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;

        debugCreateInfo.pfnUserCallback = &debug_utils_callback;
        debugCreateInfo.pUserData = nullptr;
    }

    uint32_t availableExtensionCount{ 0 };
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

    /// log available extensions
    VULKAN_DEBUG("Available Instance Extensions:");
    for( auto& ext : availableExtensions )
        VULKAN_DEBUG("\t{}", std::string(ext.extensionName).c_str());

    for( const char* extension : enabledExtensions )
    {
        bool foundExtension{ false };

        for( const auto& extensionProperty : availableExtensions )
        {
            if( strcmp(extension, extensionProperty.extensionName) == 0 )
            {
                foundExtension = true;
                break;
            }
        }

        if( !foundExtension )
        {
            VULKAN_ERROR("Extension {} requested but not available", extension);
            QUITFMT("Requested extension that is not available. See above for details.");
        }
    }

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = engineName.c_str();
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 2);
    appInfo.apiVersion = apiVersion;

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = to_u32(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    if( !enabledValidationLayers.empty() )
    {
        createInfo.enabledLayerCount = to_u32(enabledValidationLayers.size());
        createInfo.ppEnabledLayerNames = enabledValidationLayers.data();
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_handle);
    VK_CHECK(result, "Failed to create VkInstance.");

    if( !enabledValidationLayers.empty() )
    {
        VkResult debugResult = create_debug_utils_messenger(m_handle, &debugCreateInfo, nullptr, &m_debugUtilsMessenger);
        VK_CHECK(debugResult, "Failed to create debug utils messenger.");
    }

    query_gpus();
}

Instance::~Instance()
{
    if( m_debugUtilsMessenger != VK_NULL_HANDLE )
    {
        destroy_debug_utils_messenger(m_handle, m_debugUtilsMessenger, nullptr);
    }
    vkDestroyInstance(m_handle, nullptr);
}

void Instance::query_gpus()
{
    uint32_t deviceCount{ 0 };

    vkEnumeratePhysicalDevices(m_handle, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(m_handle, &deviceCount, physicalDevices.data());

    /// log found gpus
    VULKAN_INFO("Available GPUs:");
    m_gpus.reserve(deviceCount);
    for( uint32_t i = 0; i < deviceCount; i++ )
    {
        m_gpus.push_back(std::make_unique<PhysicalDevice>(*this, physicalDevices[i]));
    }
}

PhysicalDevice& Instance::get_first_gpu() const
{
    if( m_gpus.size() == 0 )
        QUITFMT("No GPUs available that support Vulkan.");

    for( size_t i = 0; i < m_gpus.size(); i++ )
    {
        if( m_gpus[i]->get_properties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
            return *m_gpus[i];
    }

    QUITFMT("No discrete GPU detected.");
}

VkInstance Instance::get_handle() const
{
    return m_handle;
}

uint32_t Instance::get_api_version() const
{
    return m_apiVersion;
}

const std::vector<const char*>& Instance::get_enabled_extensions() const
{
    return m_enabledExtensions;
}

bool Instance::is_extension_enabled(const char* extension) const
{
    auto it = std::find(m_enabledExtensions.begin(), m_enabledExtensions.end(), extension);
    return it != m_enabledExtensions.end();
}

} // vk