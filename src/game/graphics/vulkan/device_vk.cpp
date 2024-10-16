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
    VkDebugUtilsMessengerEXT dbgMessenger = (VkDebugUtilsMessengerEXT)m_debugger.get_impl_ptr();
    if( dbgMessenger )
    {
        destroy_debug_utils_messenger(m_instance, dbgMessenger, nullptr);
    }

    vkDestroyInstance(m_instance, nullptr);
}

u32 device_vk::initialise(u32 gpuIdx, void* surface)
{
    GFX_ASSERT(m_instance, "Cannot initialise device, there is no instance.");
    m_gpu = enumerate_gpus()[gpuIdx];

    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)m_gpu.get_impl_ptr();
    VkSurfaceKHR surfaceKHR = (VkSurfaceKHR)surface;

    // Setup all our queues.
    u32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, familyProperties.data());

    std::vector<VkDeviceQueueCreateInfo> queueInfos(familyCount, { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
    std::vector<std::vector<float>> queuePriorities(familyCount);
    m_queueFamilies = std::vector<queue_family>(familyCount);

    for( u32 familyIdx = 0; familyIdx < familyCount; familyIdx++ )
    {
        queue_family& family = m_queueFamilies[familyIdx];
        family.properties = familyProperties[familyIdx];
        family.index = familyIdx;
        family.queues = std::vector<VkQueue>(family.properties.queueCount);
        family.flags = 0;

        if( surfaceKHR )
        {
            VkBool32 supportsPresent = VK_FALSE;
            VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIdx, surfaceKHR, &supportsPresent);
            GFX_ASSERT(presentResult, "Failed to get surface support for physical device.");

            if( supportsPresent )
            {
                family.flags |= queue_family_flag_bit::supports_present;
            }
        }

        queuePriorities[familyIdx] = std::vector<float>(familyProperties[familyIdx].queueCount, 0.5f);

        VkDeviceQueueCreateInfo& createInfo = queueInfos[familyIdx];
        createInfo.queueFamilyIndex = familyIdx;
        createInfo.queueCount = familyProperties[familyIdx].queueCount;
        createInfo.pQueuePriorities = queuePriorities[familyIdx].data();
    }

    // Setup extensions.
    u32 extCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extCount);
    m_availableDeviceExtensions.resize(extCount);

    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extensions.data());

    for( u64 i = 0; i < extensions.size(); i++ )
    {
        m_availableDeviceExtensions[i] = extensions[i].extensionName;
    }

    for( const char* reqExt : get_device_extensions() )
    {
        bool enabled = false;
        for( std::string& ext : m_availableDeviceExtensions )
        {
            if( !strcmp(reqExt, ext.c_str()) )
            {
                m_enabledDeviceExtensions.push_back(reqExt);
                enabled = true;
                break;
            }
        }

        GFX_ASSERT(enabled, "Device extension is not available.");
    }

    // Create device
    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.queueCreateInfoCount = u32_cast(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = u32_cast(m_enabledDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_enabledDeviceExtensions.data();
    createInfo.pEnabledFeatures = 0; // TODO

    VkResult deviceResult = vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_device);
    switch( deviceResult )
    {
        case VK_SUCCESS:
            break;
        default:
            GFX_FATAL("Failed to create vulkan device.");
            return 1;
    }

    m_surface = surface;

    // Create the queues.
    for( queue_family& family : m_queueFamilies )
    {
        for( u32 idx = 0; idx < u32_cast(family.queues.size()); idx++ )
        {
            vkGetDeviceQueue(m_device, family.index, idx, &family.queues[idx]);
        }
    }

    // Create allocator
#ifdef GFX_VK_VMA_ALLOCATOR
    m_allocator.initialise(m_instance, (VkPhysicalDevice)m_gpu.get_impl_ptr(), m_device);
#endif

    return 0;
}

u32 device_vk::initialise(u32 gpuIdx)
{
    return initialise(gpuIdx, nullptr);
}

void device_vk::shutdown()
{
    m_allocator.shutdown();

    if( m_surface )
    {
        vkDestroySurfaceKHR(m_instance, (VkSurfaceKHR)m_surface, nullptr);
    }

    vkDestroyDevice(m_device, nullptr);

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

        out[i].set_impl_ptr(device);
    }
    return out;
}

buffer device_vk::create_buffer(u64 size, buffer_usage usage, memory_type mem_type, bool persistant)
{
    memory_info memoryInfo{ };
    memoryInfo.size = size;
    memoryInfo.offset = 0;
    memoryInfo.type = static_cast<u32>(mem_type);
    memoryInfo.persistant = static_cast<u32>(persistant);

    void* pImpl = nullptr;
#ifdef GFX_VK_VMA_ALLOCATOR
    vma_allocation<VkBuffer> allocation = m_allocator.allocate_buffer(size, usage, mem_type);
    memoryInfo.backing_memory = allocation.allocation;
    pImpl = allocation.resource;
#endif

    if( persistant )
    {
    #ifdef GFX_VK_VMA_ALLOCATOR
        memoryInfo.mapped = m_allocator.map((VmaAllocation)memoryInfo.backing_memory);
    #endif
    }

    return buffer(memoryInfo, usage, pImpl);
}

void device_vk::map(buffer* buf)
{
    GFX_ASSERT(!buf->get_allocation().persistant, "Persistant buffer cannot be manually mapped.");
    GFX_ASSERT(static_cast<memory_type>(buf->get_allocation().type) == memory_type::cpu_accessible, "Cannot map non-cpu accessible memory.");
    
#ifdef GFX_VK_VMA_ALLOCATOR
    buf->get_allocation().mapped = m_allocator.map((VmaAllocation)buf->get_allocation().backing_memory);
#endif
}

void device_vk::unmap(buffer* buf)
{
    GFX_ASSERT(!buf->get_allocation().persistant, "Persistant buffer cannot be manually unmapped.");
    GFX_ASSERT(!buf->get_allocation().mapped, "Persistant buffer cannot be manually unmapped.");

#ifdef GFX_VK_VMA_ALLOCATOR
    m_allocator.unmap((VmaAllocation)buf->get_allocation().backing_memory);
#endif
}

void device_vk::free_buffer(buffer* buf)
{
    GFX_ASSERT(buf->get_allocation().persistant || !buf->get_allocation().mapped, "Buffer should be unmapped before freeing.");

    if( buf->get_allocation().persistant )
    {
    #ifdef GFX_VK_VMA_ALLOCATOR
        m_allocator.unmap((VmaAllocation)buf->get_allocation().backing_memory);
    #endif
    }

#ifdef GFX_VK_VMA_ALLOCATOR
    m_allocator.free_buffer({ (VkBuffer)buf->get_impl_ptr(), (VmaAllocation)buf->get_allocation().backing_memory });
#endif
}

void device_vk::wait_idle()
{
    vkDeviceWaitIdle(m_device);
}

VkQueue device_vk::get_queue(u32 familyIdx, u32 idx) const
{
    GFX_ASSERT(u32_cast(m_queueFamilies.size()) > familyIdx, "Family index is invalid.");

    const queue_family& family = m_queueFamilies[familyIdx];
    GFX_ASSERT(u32_cast(family.queues.size()) > idx, "Queue index is invalid.");

    return family.queues[idx];
}

VkQueue device_vk::get_queue_by_flags(VkQueueFlags flags, u32 idx) const
{
    for( const queue_family& family : m_queueFamilies )
    {
        if( (family.properties.queueFlags & flags) == flags )
        {
            GFX_ASSERT(u32_cast(family.queues.size() > idx), "Queue index is invalid.");
            return family.queues[idx];
        }
    }

    GFX_ASSERT(false, "Queue family flags are not valid.");
    return VK_NULL_HANDLE;
}

VkQueue device_vk::get_queue_by_present(u32 idx) const
{
    for( const queue_family& family : m_queueFamilies )
    {
        if( family.flags & queue_family_flag_bit::supports_present )
        {
            GFX_ASSERT(u32_cast(family.queues.size() > idx), "Queue index is invalid.");
            return family.queues[idx];
        }
    }

    GFX_ASSERT(false, "Unable to find queue with present support.");
    return VK_NULL_HANDLE;
}

u32 device_vk::get_family_index_by_flags(VkQueueFlags flags) const
{
    for( u64 i = 0; i < m_queueFamilies.size(); i++ )
    {
        const queue_family& family = m_queueFamilies[i];
        if( (family.properties.queueFlags & flags) == flags )
        {
            return u32_cast(i);
        }
    }

    return u32_max;
}

bool device_vk::queue_family_supports_present(u32 familyIdx) const
{
    GFX_ASSERT(u32_cast(m_queueFamilies.size()) < familyIdx, "Family index is invalid.");
    return m_queueFamilies[familyIdx].flags & queue_family_flag_bit::supports_present;
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
    m_availableInstanceLayers.resize(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    for( u64 i = 0; i < layers.size(); i++ )
    {
        m_availableInstanceLayers[i] = layers[i].layerName;
    }

    std::vector<const char*> enabledLayers;
    for( const char* reqLayer : get_instance_layers() )
    {
        bool enabled = false;
        for( std::string& lyr : m_availableInstanceLayers )
        {
            if( !strcmp(reqLayer, lyr.c_str()) )
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
    m_availableInstanceExtensions.resize(extCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extensions.data());

    for( u64 i = 0; i < extensions.size(); i++ )
    {
        m_availableInstanceExtensions[i] = extensions[i].extensionName;
    }

    std::vector<const char*> enableExtensions;
    for( const char* reqExt : get_instance_extensions() )
    {
        bool enabled = false;
        for( std::string& ext : m_availableInstanceExtensions )
        {
            if( !strcmp(reqExt, ext.c_str()) )
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

    VkDebugUtilsMessengerEXT dbgMessenger{ };
    VkResult dbgResult = create_debug_utils_messenger(m_instance, &dbgCreateInfo, nullptr, &dbgMessenger);
    switch( dbgResult )
    {
        case VK_SUCCESS:
            break;
        default:
            break;
    }

    m_debugger.set_impl_ptr(dbgMessenger);
}

std::vector<const char*> device_vk::get_instance_extensions() const
{
    return { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
}

std::vector<const char*> device_vk::get_instance_layers() const
{
    return { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };
}

std::vector<const char*> device_vk::get_device_extensions() const
{
    return { };
}

void device_vk::dump_info() const
{
    GFX_VERBOSE("Instance extensions:");
    for( const char* ext : m_enabledInstanceExtensions )
    {
        GFX_VERBOSE("\t*{}", ext);
    }
    for( const std::string& ext : m_availableInstanceExtensions )
    {
        bool output = true;
        for( const char* ext2 : m_enabledInstanceExtensions )
        {
            if( !strcmp(ext.c_str(), ext2) )
            {
                output = false;
                break;
            }
        }

        if( output )
        {
            GFX_VERBOSE("\t{}", ext);
        }
    }


    GFX_VERBOSE("Instance layers:");
    for( const char* lyr : m_enabledInstanceLayers )
    {
        GFX_VERBOSE("\t*{}", lyr);
    }
    for( const std::string& lyr : m_availableInstanceLayers )
    {
        bool output = true;
        for( const char* lyr2 : m_enabledInstanceLayers )
        {
            if( !strcmp(lyr.c_str(), lyr2) )
            {
                output = false;
                break;
            }
        }

        if( output )
        {
            GFX_VERBOSE("\t{}", lyr);
        }
    }


    GFX_VERBOSE("Device extensions:");
    for( const char* ext : m_enabledDeviceExtensions )
    {
        GFX_VERBOSE("\t*{}", ext);
    }
    for( const std::string& ext : m_availableDeviceExtensions )
    {
        bool output = true;
        for( const char* ext2 : m_enabledDeviceExtensions )
        {
            if( !strcmp(ext.c_str(), ext2) )
            {
                output = false;
                break;
            }
        }

        if( output )
        {
            GFX_VERBOSE("\t{}", ext);
        }
    }
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