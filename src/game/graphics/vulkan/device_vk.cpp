#include "device_vk.h"

namespace gfx
{

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                    VkDebugUtilsMessageTypeFlagsEXT,
                                                    const VkDebugUtilsMessengerCallbackDataEXT*,
                                                    void*);

VkResult create_debug_utils_messenger(VkInstance,
                                      const VkDebugUtilsMessengerCreateInfoEXT*,
                                      const VkAllocationCallbacks*,
                                      VkDebugUtilsMessengerEXT*);

void destroy_debug_utils_messenger(VkInstance,
                                   VkDebugUtilsMessengerEXT,
                                   const VkAllocationCallbacks*);

device_vk::device_vk()
{
    create_instance();
}

device_vk::~device_vk()
{
    VkDebugUtilsMessengerEXT* dbgMessenger = (VkDebugUtilsMessengerEXT*)m_debugger.get_impl_ptr();
    if( dbgMessenger )
    {
        destroy_debug_utils_messenger(m_instance, *dbgMessenger, nullptr);
    }

    vkDestroyInstance(m_instance, nullptr);
}

u32 device_vk::initialise(u32 gpuIdx)
{
    return 0;
}

void device_vk::shutdown()
{
    return;
}

std::vector<gpu> device_vk::enumerate_gpus() const
{
    GFX_ASSERT(m_instance, "VkInstance is not valid.");

    u32 count = 0;
    vkEnumeratePhysicalDevices(m_instance, &count, nullptr);

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(m_instance, &count, devices.data());

    std::vector<gpu> out;
    for( u32 i = 0; i < count; i++ )
    {
        VkPhysicalDevice device = devices[i];

        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceFeatures(device, &features);
        vkGetPhysicalDeviceProperties(device, &properties);
        vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

        u64 totalSize = 0;
        for( u64 heapIdx = 0; heapIdx < memProperties.memoryHeapCount; heapIdx++ )
        {
            totalSize += memProperties.memoryHeaps[heapIdx].size;
        }

        out.emplace_back(properties.deviceName,
                         i,
                         totalSize,
                         properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    }
    return out;
}

void device_vk::wait_idle()
{
    vkDeviceWaitIdle(m_device);
}

void device_vk::create_instance()
{
    m_debugger.attach_callback([](debugger::severity level, const char* message)
        {
            switch( level )
            {
                case debugger::severity::verbose:
                    GFX_VERBOSE(message);
                    break;
                case debugger::severity::info:
                    GFX_INFO(message);
                    break;
                case debugger::severity::warning:
                    GFX_WARN(message);
                    break;
                case debugger::severity::error:
                    GFX_ERROR(message);
                    GFX_ASSERT(false, "Validation failure, see previous errors.");
                    break;
                default:
                    GFX_MSG(message);
                    break;
            }
        });

    // Instance layers.
    u32 layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    GFX_VERBOSE("Available Instance Layers:");
    int count2 = 0;
    for( VkLayerProperties& lyr : layers )
    {
        GFX_VERBOSE("\t{}", lyr.layerName);
        count2++;
    }

    std::vector<const char*> enabledLayers;
    for( const char* reqLayer : get_instance_layers() )
    {
        bool enabled = false;
        for( VkLayerProperties& lyr : layers )
        {
            if( !strcmp(reqLayer, lyr.layerName) )
            {
                enabledLayers.push_back(reqLayer);
                enabled = true;
                break;
            }
        }

        GFX_ASSERT(enabled, "Requested layer {} is not available.", reqLayer);
    }

    // Instance extensions
    u32 extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extensions.data());

    GFX_VERBOSE("Available Instance Extensions:");
    int count = 0;
    for( VkExtensionProperties& ext : extensions )
    {
        GFX_VERBOSE("\t{}", ext.extensionName);
        count++;
    }

    std::vector<const char*> enableExtensions;
    for( const char* reqExt : get_instance_extensions() )
    {
        bool enabled = false;
        for( VkExtensionProperties& ext : extensions )
        {
            if( !strcmp(reqExt, ext.extensionName) )
            {
                enableExtensions.push_back(reqExt);
                enabled = true;
                break;
            }
        }

        GFX_ASSERT(enabled, "Requested extension {} is not available.", reqExt);
    }

    VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    dbgCreateInfo.messageSeverity = 0
        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    dbgCreateInfo.messageType = 0
        | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    dbgCreateInfo.pfnUserCallback = &debug_utils_callback;
    dbgCreateInfo.pUserData = &m_debugger;

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "gfxLib";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "gfx";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = u32_cast(enableExtensions.size());
    createInfo.ppEnabledExtensionNames = enableExtensions.data();
    createInfo.enabledLayerCount = u32_cast(enabledLayers.size());
    createInfo.ppEnabledLayerNames = enabledLayers.data();
    createInfo.pNext = &dbgCreateInfo;

    // Validation layers.

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    switch( result )
    {
        case VK_SUCCESS:
            break;
        default:
            break;
    }

    m_enabledInstanceExtensions = std::move(enableExtensions);
    m_enabledInstanceLayers = std::move(enabledLayers);

    VkDebugUtilsMessengerEXT* dbgMessenger = new VkDebugUtilsMessengerEXT();
    VkResult dbgResult = create_debug_utils_messenger(m_instance, &dbgCreateInfo, nullptr, dbgMessenger);
    switch( dbgResult )
    {
        case VK_SUCCESS:
            break;
        default:
            break;
    }

    m_debugger.set_impl_ptr(dbgMessenger);
    // Set debug messenger.
}

std::vector<const char*> device_vk::get_instance_extensions() const
{
    return { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
}

std::vector<const char*> device_vk::get_instance_layers() const
{
    return { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT type,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* pUserData)
{
    GFX_ASSERT(pUserData, "Debugger hasn't been set.");

    debugger* dbgr = static_cast<debugger*>(pUserData);
    debugger::severity level = debugger::severity::none;
    if( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
        level = debugger::severity::error;
    else if( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
        level = debugger::severity::warning;
    else if( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
        level = debugger::severity::info;
    else if( severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT )
        level = debugger::severity::verbose;

    dbgr->send_event(level, pCallbackData->pMessage);

    return VK_FALSE;
}

VkResult create_debug_utils_messenger(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if( func != nullptr )
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if( func != nullptr )
    {
        func(instance, debugMessenger, pAllocator);
    }
}


} // gfx