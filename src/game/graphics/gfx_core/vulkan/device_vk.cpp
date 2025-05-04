#include "device_vk.h"

#ifdef GFX_SUPPORTS_VULKAN
#include "vkconverts.h"

#include "../driver.h"

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

u32 VK_DEVICE::initialise(u32 gpuIdx, surface_create_func surface_func)
{
    m_deviceImpl = new device_state_vk{ };
    device_state_vk& deviceState = get_impl<device_state_vk>();
    deviceState.create_instance(m_debugger);

    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    if( !surface_func(deviceState.instance, &surface) )
    {
        GFX_ASSERT(false, "Failed to create vulkan surface.");
        return -1;
    }
    m_surface = surface;

    GFX_ASSERT(deviceState.instance, "Cannot initialise device, there is no instance.");
    m_gpu = enumerate_gpus()[gpuIdx];

    VkPhysicalDevice physicalDevice = m_gpu.get_impl<VkPhysicalDevice>();
    VkSurfaceKHR surfaceKHR = (VkSurfaceKHR)surface;

    // Setup all our queues.
    u32 familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, familyProperties.data());

    std::vector<VkDeviceQueueCreateInfo> queueInfos(familyCount, { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO });
    std::vector<std::vector<float>> queuePriorities(familyCount);
    deviceState.queue_families = std::vector<queue_family>(familyCount);

    for( u32 familyIdx = 0; familyIdx < familyCount; familyIdx++ )
    {
        queue_family& family = deviceState.queue_families[familyIdx];
        family.properties = familyProperties[familyIdx];
        family.index = familyIdx;
        family.queues = std::vector<VkQueue>(family.properties.queueCount);
        family.flags = 0;

        if( surfaceKHR )
        {
            VkBool32 supportsPresent = VK_FALSE;
            VkResult presentResult = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIdx, surfaceKHR, &supportsPresent);
            GFX_ASSERT(presentResult == VK_SUCCESS, "Failed to get surface support for physical device.");

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
    deviceState.available_device_extensions.resize(extCount);

    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extensions.data());

    for( u64 i = 0; i < extensions.size(); i++ )
    {
        deviceState.available_device_extensions[i] = extensions[i].extensionName;
    }

    for( const char* reqExt : vulkan_get_device_extensions() )
    {
        bool enabled = false;
        for( std::string& ext : deviceState.available_device_extensions )
        {
            if( !strcmp(reqExt, ext.c_str()) )
            {
                deviceState.enabled_device_extensions.push_back(reqExt);
                enabled = true;
                break;
            }
        }

        GFX_ASSERT(enabled, "Device extension is not available.");
    }

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    // Create device
    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.pNext = &dynamicRenderingFeatures;
    createInfo.queueCreateInfoCount = u32_cast(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = u32_cast(deviceState.enabled_device_extensions.size());
    createInfo.ppEnabledExtensionNames = deviceState.enabled_device_extensions.data();

    VkPhysicalDeviceFeatures features{ };
    features.wideLines = true;
    features.fillModeNonSolid = true;
    createInfo.pEnabledFeatures = &features; // TODO

    VkResult deviceResult = vkCreateDevice(physicalDevice, &createInfo, nullptr, &deviceState.device);
    switch( deviceResult )
    {
        case VK_SUCCESS:
            break;
        default:
            GFX_FATAL("Failed to create vulkan device.");
            return 1;
    }

    m_surface = surfaceKHR;

    // Create the queues.
    for( queue_family& family : deviceState.queue_families )
    {
        for( u32 idx = 0; idx < u32_cast(family.queues.size()); idx++ )
        {
            vkGetDeviceQueue(deviceState.device, family.index, idx, &family.queues[idx]);
        }
    }

    // Create allocator
#ifdef GFX_VK_VMA_ALLOCATOR
    deviceState.allocator.initialise(deviceState.instance, m_gpu.get_impl<VkPhysicalDevice>(), deviceState.device);
#endif

    // Create command pools
    deviceState.command_pool.initialise(&deviceState);

    return 0;
}

u32 VK_DEVICE::initialise(u32 gpuIdx)
{
    return initialise(gpuIdx, [](VkInstance, VkSurfaceKHR* out_surface) -> bool
        {
            *out_surface = VK_NULL_HANDLE;
            return true;
        });
}

void VK_DEVICE::shutdown()
{
    device_state_vk& deviceState = get_impl<device_state_vk>();

    deviceState.allocator.shutdown();
    deviceState.command_pool.shutdown();

    if( m_surface )
    {
        vkDestroySurfaceKHR(deviceState.instance, (VkSurfaceKHR)m_surface, nullptr);
    }

    vkDestroyDevice(deviceState.device, nullptr);

    VkDebugUtilsMessengerEXT dbgMessenger = (VkDebugUtilsMessengerEXT)m_debugger.get_impl_ptr();
    if( dbgMessenger )
    {
        destroy_debug_utils_messenger(deviceState.instance, dbgMessenger, nullptr);
    }

    vkDestroyInstance(deviceState.instance, nullptr);
    delete static_cast<device_state_vk*>(m_deviceImpl);
}

#ifdef GFX_EXT_SWAPCHAIN
surface_capabilities VK_DEVICE::get_surface_capabilities() const
{
    GFX_ASSERT(m_surface, "Surface was not provided when creating graphics device.");
    const device_state_vk& deviceState = get_impl<device_state_vk>();
    VkSurfaceKHR surface = (VkSurfaceKHR)m_surface;

    VkSurfaceCapabilitiesKHR surfaceCapabilities{ };
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu.get_impl<VkPhysicalDevice>(), surface, &surfaceCapabilities);

    u32 formatCount{ 0 };
    VkResult formatResult = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu.get_impl<VkPhysicalDevice>(), surface, &formatCount, nullptr);
    GFX_ASSERT(formatResult == VK_SUCCESS, "Failed to find supported surface formats.");

    std::vector<VkSurfaceFormatKHR> formats;
    formats.resize(formatCount);
    formatResult = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu.get_impl<VkPhysicalDevice>(), surface, &formatCount, formats.data());
    GFX_ASSERT(formatResult == VK_SUCCESS, "Failed to find supported surface formats.");

    u32 presentModeCount{ 0 };
    VkResult presentResult = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu.get_impl<VkPhysicalDevice>(), surface, &presentModeCount, nullptr);
    GFX_ASSERT(presentResult == VK_SUCCESS, "Failed to find supported surface present modes.");

    std::vector<VkPresentModeKHR> presentModes;
    presentModes.resize(presentModeCount);
    presentResult = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu.get_impl<VkPhysicalDevice>(), surface, &presentModeCount, presentModes.data());
    GFX_ASSERT(presentResult == VK_SUCCESS, "Failed to find supported surface present modes.");

    std::vector<cdt::image_format> retFormats;
    retFormats.reserve(formats.size());
    for( VkSurfaceFormatKHR sf : formats )
        retFormats.push_back(converters::get_format_vk_cdt(sf.format));

    std::vector<present_mode> retPresentModes;
    retPresentModes.reserve(presentModes.size());
    for( VkPresentModeKHR pm : presentModes )
        retPresentModes.push_back((present_mode)pm);

    return surface_capabilities
    {
        retFormats,
        retPresentModes,
        (texture_usage_flags)surfaceCapabilities.supportedUsageFlags,
        surfaceCapabilities.minImageCount,
        surfaceCapabilities.maxImageCount
    };
}

swapchain VK_DEVICE::create_swapchain(swapchain* previous, texture_info info, u32 image_count, texture_usage_flags usage, format format, present_mode present)
{
    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.oldSwapchain = previous
        ? previous->get_impl<VkSwapchainKHR>()
        : VK_NULL_HANDLE;
    createInfo.minImageCount = image_count;
    createInfo.imageExtent = { info.get_width(), info.get_height() };

    // TODO
    createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageFormat = converters::get_format_vk(format);
    createInfo.imageArrayLayers = info.get_depth();

    if( usage & TEXTURE_USAGE_SWAPCHAIN_OWNED )
        usage ^= TEXTURE_USAGE_SWAPCHAIN_OWNED;
    createInfo.imageUsage = (VkImageUsageFlags)usage;

    //TODO
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    
    createInfo.presentMode = (VkPresentModeKHR)present;
    createInfo.surface = (VkSurfaceKHR)m_surface;
    
    VkSwapchainKHR retSwapchain{ VK_NULL_HANDLE };

    device_state_vk& deviceState = get_impl<device_state_vk>();
    VkResult result = vkCreateSwapchainKHR(deviceState.device, &createInfo, nullptr, &retSwapchain);
    GFX_ASSERT(result != VK_ERROR_INITIALIZATION_FAILED, "Failed to create graphics swapchain. Ensure that '{}' device extension is enabled.", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    GFX_ASSERT(result == VK_SUCCESS, "Failed to create graphics swapchain.");

    u32 imageCount{ 0 };
    vkGetSwapchainImagesKHR(deviceState.device, retSwapchain, &imageCount, nullptr);

    std::vector<VkImage> images;
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(deviceState.device, retSwapchain, &imageCount, images.data());

    std::vector<texture> textures;
    textures.reserve(imageCount);

    for( u32 idx = 0; idx < imageCount; idx++ )
    {
        VkImage swapchainImage = images[idx];

        texture& swapchainTexture = textures.emplace_back(info);

        driver::create_swapchain_texture(
            &swapchainTexture,
            memory_info::create_as_texture(info.get_width() * info.get_height() * info.get_depth(), format, MEMORY_TYPE_GPU_ONLY, usage),
            swapchainImage);
    }

    swapchain retval;
    retval.initialise(std::move(textures), retSwapchain);
    return retval;
}

void VK_DEVICE::free_swapchain(swapchain* swapchain)
{
    vkDestroySwapchainKHR(get_impl<device_state_vk>().device, swapchain->get_impl<VkSwapchainKHR>(), nullptr);
}
#endif // GFX_EXT_SWAPCHAIN

screen_capabilities VK_DEVICE::query_screen_capabilities() const
{
    if( !m_surface )
        return { };

    VkSurfaceCapabilitiesKHR surfaceCapabilities{ };
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu.get_impl<VkPhysicalDevice>(), (VkSurfaceKHR)m_surface, &surfaceCapabilities);
    
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to query for surface capabilities.");
    }

    return screen_capabilities
    {
        .min_image_count = surfaceCapabilities.minImageCount,
        .max_image_count = surfaceCapabilities.maxImageCount,
        .current_width = surfaceCapabilities.currentExtent.width,
        .current_height = surfaceCapabilities.currentExtent.height,
        .min_width = surfaceCapabilities.minImageExtent.width,
        .min_height = surfaceCapabilities.minImageExtent.height,
        .max_width = surfaceCapabilities.maxImageExtent.width,
        .max_height = surfaceCapabilities.maxImageExtent.height
    };
}

std::vector<gpu> VK_DEVICE::enumerate_gpus() const
{
    const device_state_vk& deviceState = get_impl<device_state_vk>();
    GFX_ASSERT(deviceState.instance, "VkInstance is not valid.");

    u32 count = 0;
    vkEnumeratePhysicalDevices(deviceState.instance, &count, nullptr);

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(deviceState.instance, &count, devices.data());

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

        out.push_back({ });
        out.back().initialise(properties.deviceName,
                              i,
                              totalSize,
                              properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                              device);
    }
    return out;
}

void* VK_DEVICE::allocate_buffer(resource* resource, const memory_info& memory_info)
{
    GFX_ASSERT(resource, "Resource must not be nullptr.");
    GFX_ASSERT(resource->get_backing_memory<void*>() == nullptr, "Resource has already been allocated.");

#ifdef GFX_VK_VMA_ALLOCATOR
    vma_allocation<VkBuffer> allocation =
        get_impl<device_state_vk>().allocator.allocate_buffer(memory_info);
    resource->initialise(memory_info, allocation.allocation);
    return allocation.resource;
#endif
}

void VK_DEVICE::free_buffer(buffer* buffer)
{
    VmaAllocation allocation = buffer->get_backing_memory<VmaAllocation>();
    VkBuffer vk_buffer = buffer->get_impl<VkBuffer>();
    if( allocation == nullptr && vk_buffer == VK_NULL_HANDLE )
        return;

    GFX_ASSERT(!buffer->is_mapped(), "Buffer should be unmapped before freeing.");
    device_state_vk& deviceState = get_impl<device_state_vk>();

#ifdef GFX_VK_VMA_ALLOCATOR
    deviceState.allocator.free_buffer({ vk_buffer, allocation });
#endif
}

void* VK_DEVICE::create_buffer_view_impl(buffer_view* view,  buffer_view_range range)
{
    VkBufferViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };
    createInfo.buffer = view->get_resource()->get_impl<VkBuffer>();
    createInfo.format = converters::get_format_vk(view->get_format());
    createInfo.offset = range.offset;
    createInfo.range = range.size;

    VkBufferView retval{ VK_NULL_HANDLE };
    VkResult result = vkCreateBufferView(get_impl<device_state_vk>().device, &createInfo, nullptr, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create Buffer View.");
        break;
    }

    return retval;
}

void VK_DEVICE::destroy_buffer_view_impl(buffer_view* view)
{
    vkDestroyBufferView(get_impl<device_state_vk>().device, view->get_impl<VkBufferView>(), nullptr);
}

void* VK_DEVICE::allocate_texture(texture* texture, const memory_info& memory_info, resource_view_type view_type)
{
    GFX_ASSERT(texture, "Resource must not be nullptr.");
    GFX_ASSERT(texture->get_backing_memory<void*>() == nullptr, "Texture has already been allocated.");

#ifdef GFX_VK_VMA_ALLOCATOR
    vma_allocation<VkImage> allocation = 
        get_impl<device_state_vk>().allocator.allocate_image(memory_info, *static_cast<texture_info*>(texture), memory_info.get_format(), view_type, texture->get_layout());
    static_cast<resource*>(texture)->initialise(memory_info, allocation.allocation);
    return allocation.resource;
#endif
}

void VK_DEVICE::free_texture(texture* texture)
{
    VmaAllocation allocation = texture->get_backing_memory<VmaAllocation>();
    VkImage vk_image = texture->get_impl<VkImage>();
    if( texture->is_swapchain_owned() || (allocation == nullptr && vk_image == VK_NULL_HANDLE) )
        return;

    GFX_ASSERT(!texture->is_mapped(), "Texture should be unmapped before freeing.");

#ifdef GFX_VK_VMA_ALLOCATOR
    get_impl<device_state_vk>().allocator.free_image({ vk_image, allocation });
#endif
}

void* VK_DEVICE::create_texture_view_impl(texture_view* view, texture_view_range range)
{
    VkImageView pImpl{ VK_NULL_HANDLE };
    VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    createInfo.image = view->get_resource()->get_impl<VkImage>();
    createInfo.format = converters::get_format_vk(view->get_format());
    createInfo.viewType = converters::get_view_type_vk(view->get_type());

    VkImageSubresourceRange subresource{ };
    subresource.aspectMask = has_depth_channel(view->get_format())
        ? VK_IMAGE_ASPECT_DEPTH_BIT
        : VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseMipLevel = u32_cast(range.base_mip);
    subresource.baseArrayLayer = u32_cast(range.base_layer);
    subresource.levelCount = u32_cast(range.mip_count);
    subresource.layerCount = u32_cast(range.layer_count);

    createInfo.subresourceRange = subresource;

    VkResult result = vkCreateImageView(get_impl<device_state_vk>().device, &createInfo, nullptr, &pImpl);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create Image View.");
        break;
    }

    return pImpl;
}

void VK_DEVICE::destroy_texture_view_impl(texture_view* view)
{
    vkDestroyImageView(get_impl<device_state_vk>().device, view->get_impl<VkImageView>(), nullptr);
}

void* VK_DEVICE::create_texture_sampler_impl(texture_sampler* sampler)
{
    // TODO all of this shit, should be pretty easy to add into texture_sampler.
    VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.minFilter = VK_FILTER_NEAREST;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.anisotropyEnable = VK_FALSE;
    createInfo.maxAnisotropy = 0;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    
    VkSampler retval{ VK_NULL_HANDLE };
    VkResult result = vkCreateSampler(get_impl<device_state_vk>().device, &createInfo, nullptr, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create sampler.");
        break;
    }

    return retval;
}

void VK_DEVICE::destroy_texture_sampler_impl(texture_sampler* sampler)
{
    vkDestroySampler(get_impl<device_state_vk>().device, sampler->get_impl<VkSampler>(), nullptr);
}

fence VK_DEVICE::create_fence(bool signaled)
{
    VkFence impl{ VK_NULL_HANDLE };
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    VkResult result = vkCreateFence(get_impl<device_state_vk>().device, &createInfo, nullptr, &impl);

    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Fence creation failed.");
    }

    return fence(impl);
}

void VK_DEVICE::free_fence(fence* fence)
{
    vkDestroyFence(get_impl<device_state_vk>().device, fence->get_impl<VkFence>(), nullptr);
}

dependency VK_DEVICE::create_dependency(const char* debug_name)
{
    dependency retval{ };
    VkSemaphore retSemaphore{ VK_NULL_HANDLE };

    VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkResult result = vkCreateSemaphore(get_impl<device_state_vk>().device, &createInfo, nullptr, &retSemaphore);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create Semaphore.");
    }

    retval.initialise(retSemaphore, debug_name);
    return retval;
}

void VK_DEVICE::free_dependency(dependency* dep)
{
    vkDestroySemaphore(get_impl<device_state_vk>().device, dep->get_impl<VkSemaphore>(), nullptr);
}

graphics_command_list VK_DEVICE::allocate_graphics_command_list(bool secondary)
{
    VkCommandBuffer buffer = get_impl<device_state_vk>().command_pool.allocate_buffer_by_flags(VK_QUEUE_GRAPHICS_BIT, secondary);
    graphics_command_list retval{ };
    retval.init(buffer, secondary);
    return retval;
}

void VK_DEVICE::free_command_list(command_list* list)
{
    VkQueueFlags flags{ };
    switch( list->get_type() )
    {
    case command_list_type::graphics:
        flags = VK_QUEUE_GRAPHICS_BIT;
        break;
    default:
        break;
    }

    get_impl<device_state_vk>().command_pool.free_buffer_by_flags(list->get_impl<VkCommandBuffer>(), flags);
}

u8* VK_DEVICE::map_resource(const resource* resource)
{
    GFX_ASSERT(resource->get_memory_type() == MEMORY_TYPE_CPU_VISIBLE, "Cannot map non-cpu accessible memory.");

#ifdef GFX_VK_VMA_ALLOCATOR
    return get_impl<device_state_vk>().allocator.map(resource->get_backing_memory<VmaAllocation>());
#endif
}

void VK_DEVICE::unmap_resource(const resource* resource)
{
    GFX_ASSERT(resource->is_mapped(), "Resource that isn't mapped cannot be unmapped.");

#ifdef GFX_VK_VMA_ALLOCATOR
    get_impl<device_state_vk>().allocator.unmap(resource->get_backing_memory<VmaAllocation>());
#endif
}

void VK_DEVICE::wait_idle()
{
    vkDeviceWaitIdle(get_impl<device_state_vk>().device);
}

#ifdef GFX_EXT_SWAPCHAIN
swapchain_acquire_result VK_DEVICE::acquire_next_image(swapchain* swapchain, u32* aquired_index, dependency* signal_dep, fence* signal_fence, u64 timeout)
{
    GFX_ASSERT(signal_dep || signal_fence, "Must provide either a dependency or a fence to signal.");

    VkFence signalFence{ VK_NULL_HANDLE };
    if( signal_fence )
    {
        signalFence = signal_fence->get_impl<VkFence>();
    }

    VkSemaphore signalSemaphore{ VK_NULL_HANDLE };
    if( signal_dep )
    {
        signalSemaphore = signal_dep->get_impl<VkSemaphore>();
    }

    VkResult result = vkAcquireNextImageKHR(get_impl<device_state_vk>().device, swapchain->get_impl<VkSwapchainKHR>(), timeout, signalSemaphore, signalFence, aquired_index);

    switch( result )
    {
    case VK_SUCCESS:
        return SWAPCHAIN_ACQUIRE_SUCCESS;
    case VK_SUBOPTIMAL_KHR:
        return SWAPCHAIN_ACQUIRE_SUBOPTIMAL;
    case VK_ERROR_OUT_OF_DATE_KHR:
        return SWAPCHAIN_ACQUIRE_OUT_OF_DATE;
    default:
        GFX_ASSERT(false, "Failed to acquire next swapchain image.");
    }

    return SWAPCHAIN_ACQUIRE_OUT_OF_DATE;
}

void VK_DEVICE::present(swapchain* swapchain, u32 image_index, const std::vector<dependency*>& dependencies)
{
    VkSwapchainKHR sc = swapchain->get_impl<VkSwapchainKHR>();
    VkResult result;

    VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &sc;
    presentInfo.pImageIndices = &image_index;
    presentInfo.pResults = &result;

    std::vector<VkSemaphore> waitSema;
    if( !dependencies.empty() )
    {
        waitSema.reserve(dependencies.size());
        for( const dependency* dep : dependencies )
        {
            waitSema.push_back(dep->get_impl<VkSemaphore>());
        }

        presentInfo.waitSemaphoreCount = u32_cast(waitSema.size());
        presentInfo.pWaitSemaphores = waitSema.data();
    }

    vkQueuePresentKHR(get_impl<device_state_vk>().get_queue_by_present(), &presentInfo);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Error presenting swapchain image index {}", image_index);
        break;
    }
}
#endif // GFX_EXT_SWAPCHAIN

bool VK_DEVICE::wait_for_fences(const fence* pFences, u32 count, bool wait_for_all, u64 timeout) const
{
    VkFence* fences = new VkFence[count];
    for( u32 i = 0; i < count; i++ )
    {
        fences[i] = pFences[i].get_impl<VkFence>();
    }

    VkResult result = vkWaitForFences(get_impl<device_state_vk>().device, count, fences, wait_for_all, timeout);
    delete[] fences;

    return result == VK_SUCCESS;
}

bool VK_DEVICE::reset_fences(fence* pFences, u32 count)
{
    VkFence* fences = new VkFence[count];
    for( u32 i = 0; i < count; i++ )
    {
        fences[i] = pFences[i].get_impl<VkFence>();
    }

    VkResult result = vkResetFences(get_impl<device_state_vk>().device, count, fences);
    delete[] fences;

    return result == VK_SUCCESS;
}

bool VK_DEVICE::check_fence(const fence* fence) const
{
    VkResult result = vkGetFenceStatus(get_impl<device_state_vk>().device, fence->get_impl<VkFence>());
    return result == VK_SUCCESS;
}

void VK_DEVICE::reset(command_list* list)
{
    VkCommandBufferResetFlags flags{ };
    // flags |= VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;

    vkResetCommandBuffer(list->get_impl<VkCommandBuffer>(), flags);
}

void VK_DEVICE::begin(command_list* list)
{
    VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    // FLAGS?

    // Don't inherit anything. Think this is fine.
    VkCommandBufferInheritanceInfo inheritInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
    if( list->is_secondary() )
    {
        begin.pInheritanceInfo = &inheritInfo;
    }

    vkBeginCommandBuffer(list->get_impl<VkCommandBuffer>(), &begin);
}

void VK_DEVICE::end(command_list* list)
{
    vkEndCommandBuffer(list->get_impl<VkCommandBuffer>());
}

static void vulkan_submit_impl(VkQueue queue, const std::vector<command_list*>& lists, fence* fence)
{
    VkSubmitInfo info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    info.commandBufferCount = u32_cast(lists.size());
    std::vector<VkCommandBuffer> buffers;
    buffers.reserve(lists.size());

    for( command_list* list : lists )
    {
        buffers.push_back(list->get_impl<VkCommandBuffer>());
    }

    info.pCommandBuffers = buffers.data();
    std::vector<VkSemaphore> waitSema;
    std::vector<VkSemaphore> signalSema;
    std::vector<VkPipelineStageFlags> waitSemaStages; // TODO?

    for( const command_list* list : lists )
    {
        for( const dependency* dep : list->get_wait_dependencies() )
        {
            waitSema.push_back(dep->get_impl<VkSemaphore>());
            waitSemaStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }

        if( list->get_signal_dependency() != nullptr )
        {
            signalSema.push_back(list->get_signal_dependency()->get_impl<VkSemaphore>());
        }
    }

    info.waitSemaphoreCount = u32_cast(waitSema.size());
    info.pWaitSemaphores = waitSema.data();
    info.signalSemaphoreCount = u32_cast(signalSema.size());
    info.pSignalSemaphores = signalSema.data();

    info.pWaitDstStageMask = waitSemaStages.data(); // TODO

    VkFence submitFence = fence
        ? fence->get_impl<VkFence>()
        : VK_NULL_HANDLE;
    vkQueueSubmit(queue, 1, &info, submitFence);
}

void VK_DEVICE::submit(const std::vector<graphics_command_list*>& lists, fence* fence)
{
    VkQueue queue = get_impl<device_state_vk>().get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT);
    std::vector<command_list*> castLists;
    castLists.reserve(lists.size());

    for( graphics_command_list* list : lists )
    {
        castLists.push_back(reinterpret_cast<command_list*>(list));
    }

    vulkan_submit_impl(queue, castLists, fence);
}

void VK_DEVICE::submit(const std::vector<compute_command_list*>& lists, fence* fence)
{
    VkQueue queue = get_impl<device_state_vk>().get_queue_by_flags(VK_QUEUE_COMPUTE_BIT);
    std::vector<command_list*> castLists;
    castLists.reserve(lists.size());

    for( compute_command_list* list : lists )
    {
        castLists.push_back(reinterpret_cast<command_list*>(list));
    }

    vulkan_submit_impl(queue, castLists, fence);
}

void VK_DEVICE::submit(const std::vector<present_command_list*>& lists, fence* fence)
{
    VkQueue queue = get_impl<device_state_vk>().get_queue_by_present();
    std::vector<command_list*> castLists;
    castLists.reserve(lists.size());

    for( present_command_list* list : lists )
    {
        castLists.push_back(reinterpret_cast<command_list*>(list));
    }

    vulkan_submit_impl(queue, castLists, fence);
}

void VK_DEVICE::draw(command_list* list, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
{
    vkCmdDraw(list->get_impl<VkCommandBuffer>(), vertex_count, instance_count, first_vertex, first_instance);
}

void VK_DEVICE::draw_indexed(command_list* list, u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance)
{
    vkCmdDrawIndexed(list->get_impl<VkCommandBuffer>(), index_count, instance_count, first_index, vertex_offset, first_instance);
}

void VK_DEVICE::bind_program(command_list* list, program* prog, u64 passIdx)
{
    VkPipelineBindPoint bindPoint{ };
    switch( list->get_type() )
    {
    case command_list_type::graphics:
        bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        break;
    case command_list_type::compute:
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        break;
    }

    vkCmdBindPipeline(list->get_impl<VkCommandBuffer>(), bindPoint, prog->get_pass(passIdx).get_impl<VkPipeline>());
}

void VK_DEVICE::bind_vertex_buffers(command_list* list, buffer** pBuffers, u32 buffer_count, u32 first_vertex_index)
{
    std::vector<VkBuffer> bufs;
    std::vector<VkDeviceSize> offsets;

    for( u32 i = 0; i < buffer_count; i++ )
    {
        bufs.push_back(pBuffers[i]->get_impl<VkBuffer>());
        // TODO offset
        offsets.push_back(0);
    }

    vkCmdBindVertexBuffers(list->get_impl<VkCommandBuffer>(), first_vertex_index, buffer_count, bufs.data(), offsets.data());
}

void VK_DEVICE::bind_index_buffer(command_list* list, buffer* buffer, index_buffer_type index_type)
{
    VkIndexType type;
    switch( index_type )
    {
        case index_buffer_type::INDEX_TYPE_U16:
            type = VK_INDEX_TYPE_UINT16;
            break;
        case index_buffer_type::INDEX_TYPE_U32:
            type = VK_INDEX_TYPE_UINT32;
            break;
        default:
            GFX_ASSERT(false, "Invalid index type size when binding index buffer.");
    }

    // TODO offset
    VkDeviceSize offset = 0;
    vkCmdBindIndexBuffer(list->get_impl<VkCommandBuffer>(), buffer->get_impl<VkBuffer>(), offset, type);
}

void VK_DEVICE::bind_descriptor_tables(command_list* list, pass* pass, descriptor_table** pTables, u32 table_count, descriptor_table_type type)
{
    GFX_ASSERT(table_count > 0, "Must provide atleast one descriptor table.");

    // TODO multiple descriptor tables.
    u32 set_index = u32_cast(type);
    std::vector<VkDescriptorSet> descriptor_sets;
    for( u32 idx = 0; idx < table_count; idx++ )
    {
        descriptor_sets.push_back(pTables[idx]->get_impl<VkDescriptorSet>());
    }

    VkPipelineBindPoint bindPoint{ };
    switch( list->get_type() )
    {
    case command_list_type::graphics:
        bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        break;
    case command_list_type::compute:
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        break;
    default:
        GFX_ASSERT(false, "Command list type does not support this command.");
        break;
    }

    vkCmdBindDescriptorSets(list->get_impl<VkCommandBuffer>(), bindPoint, pass->get_layout_impl<VkPipelineLayout>(), set_index, table_count, descriptor_sets.data(), 0, nullptr);
}

void VK_DEVICE::begin_rendering(command_list* list, const std::vector<texture_attachment>& color_outputs, texture_attachment* depth_output)
{
    GFX_ASSERT(!color_outputs.empty() || depth_output, "Atleast one color output or a depth output must be provided.");

    VkRect2D renderSize{ };
    if( !color_outputs.empty() )
    {
        const texture_attachment& first = color_outputs[0];
        renderSize = { 0, 0, first.view->get_resource()->get_width(), first.view->get_resource()->get_height() };
    }
    else
    {
        renderSize = { 0, 0, depth_output->view->get_resource()->get_width(), depth_output->view->get_resource()->get_height() };
    }

    VkRenderingInfo renderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    VkRenderingAttachmentInfo depthAttachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };

    std::vector<VkRenderingAttachmentInfo> color_attachments;
    color_attachments.reserve(color_outputs.size());

    for( u32 idx = 0; idx < u32_cast(color_outputs.size()); idx++ )
    {
        const texture_attachment& output = color_outputs[idx];
        VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        attachment.imageView = output.view->get_impl<VkImageView>();
        attachment.imageLayout = converters::get_layout_vk(output.view->get_resource()->get_layout());
        attachment.loadOp = converters::get_load_op_vk(output.load);
        attachment.storeOp = converters::get_store_op_vk(output.store);

        color_attachments.push_back(attachment);
    }

    if( depth_output )
    {
        depthAttachment.imageView = depth_output->view->get_impl<VkImageView>();
        depthAttachment.imageLayout = converters::get_layout_vk(depth_output->view->get_resource()->get_layout());
        depthAttachment.loadOp = converters::get_load_op_vk(depth_output->load);
        depthAttachment.storeOp = converters::get_store_op_vk(depth_output->store);

        renderInfo.pDepthAttachment = &depthAttachment;
    }

    renderInfo.colorAttachmentCount = u32_cast(color_attachments.size());
    renderInfo.pColorAttachments = color_attachments.data();

    renderInfo.renderArea = renderSize;
    renderInfo.layerCount = 1;
    vkCmdBeginRendering(list->get_impl<VkCommandBuffer>(), &renderInfo);
}

void VK_DEVICE::end_rendering(command_list* list)
{
    vkCmdEndRendering(list->get_impl<VkCommandBuffer>());
}

void VK_DEVICE::set_viewport(command_list* list, f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth)
{
    VkViewport viewport
    {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .minDepth = min_depth,
        .maxDepth = max_depth
    };

    vkCmdSetViewport(list->get_impl<VkCommandBuffer>(), 0, 1, &viewport);
}

void VK_DEVICE::set_scissor(command_list* list, u32 x, u32 y, u32 width, u32 height)
{
    VkRect2D scissor
    {
        .offset = { i32_cast(x), i32_cast(y) },
        .extent = { width, height }
    };

    vkCmdSetScissor(list->get_impl<VkCommandBuffer>(), 0, 1, &scissor);
}

void VK_DEVICE::copy_texture_to_texture(command_list* list, texture* src, texture* dst)
{
    VkImageSubresourceLayers srcSubresource{ };
    srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO
    srcSubresource.mipLevel = 0;
    srcSubresource.baseArrayLayer = 0;
    srcSubresource.layerCount = 1;

    VkImageSubresourceLayers dstSubresource{ };
    dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO
    dstSubresource.mipLevel = 0;
    dstSubresource.baseArrayLayer = 0;
    dstSubresource.layerCount = 1;

    VkImageCopy defaultRegion{ };
    defaultRegion.srcOffset = { 0, 0, 0 };
    defaultRegion.srcSubresource = srcSubresource;
    defaultRegion.dstOffset = { 0, 0, 0 };
    defaultRegion.dstSubresource = dstSubresource;
    defaultRegion.extent = { src->get_width(), src->get_height(), src->get_depth() };

    vkCmdCopyImage(
        list->get_impl<VkCommandBuffer>(),
        src->get_impl<VkImage>(),
        converters::get_layout_vk(src->get_layout()),
        dst->get_impl<VkImage>(),
        converters::get_layout_vk(dst->get_layout()),
        1,
        &defaultRegion);
}

void VK_DEVICE::copy_buffer_to_texture(command_list* list, buffer* src, texture* dst)
{
    VkImageSubresourceLayers dstSubresource{ };
    dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO
    dstSubresource.mipLevel = 0;
    dstSubresource.baseArrayLayer = 0;
    dstSubresource.layerCount = 1;

    VkBufferImageCopy copy{ };
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;
    copy.imageSubresource = dstSubresource;
    copy.imageOffset = { 0, 0 };
    copy.imageExtent = { dst->get_width(), dst->get_height(), dst->get_depth() };

    vkCmdCopyBufferToImage(
        list->get_impl<VkCommandBuffer>(),
        src->get_impl<VkBuffer>(),
        dst->get_impl<VkImage>(),
        converters::get_layout_vk(dst->get_layout()),
        1,
        &copy);
}

void VK_DEVICE::copy_buffer_to_buffer(command_list* list, buffer* src, buffer* dst)
{
    // TOOD add some offsets and sizes here
    VkBufferCopy region{ 0, 0, src->get_size() };
    vkCmdCopyBuffer(
        list->get_impl<VkCommandBuffer>(),
        src->get_impl<VkBuffer>(),
        dst->get_impl<VkBuffer>(),
        1,
        &region);
}

void VK_DEVICE::texture_barrier(command_list* list, texture* texture, texture_layout dst_layout, pipeline_stage_flag_bits src_stage, pipeline_stage_flag_bits dst_stage)
{
    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    // Aspect masks?
    barrier.oldLayout = converters::get_layout_vk(texture->get_layout());
    barrier.newLayout = converters::get_layout_vk(dst_layout);
    barrier.image = texture->get_impl<VkImage>();
    barrier.subresourceRange =
    {
        // Aspect mask needs an abstraction
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    vkCmdPipelineBarrier(
        list->get_impl<VkCommandBuffer>(),
        converters::get_pipeline_stage_flags_vk(src_stage),
        converters::get_pipeline_stage_flags_vk(dst_stage),
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);
}

void VK_DEVICE::execute_command_lists(command_list* list, command_list** execute_lists, u32 count)
{
    std::vector<VkCommandBuffer> buffers;
    for( u32 i = 0; i < count; i++ )
    {
        buffers.push_back(execute_lists[i]->get_impl<VkCommandBuffer>());
    }

    vkCmdExecuteCommands(list->get_impl<VkCommandBuffer>(), count, buffers.data());
}

static VkPipeline vulkan_create_graphics_pipeline_impl(VkDevice device, program* program, u64 passIdx)
{
    GFX_ASSERT(program->get_pass_count() > passIdx, "Program {} ({}) does not have a pass index {}", program->get_name().get_hash(), program->get_name().try_get_str(), passIdx);
    const pass& rPass = program->get_pass(passIdx);

    GFX_ASSERT(rPass.get_layout_impl<void*>() != nullptr, "Pass layout impl must have already been set.");

    dt::vector<VkShaderModule> modules;
    dt::vector<VkPipelineShaderStageCreateInfo> stages;

    for( u32 stage = 1; stage < SHADER_STAGE_FINAL; stage <<= 1 )
    {
        shader_stage_flag_bits eStage = static_cast<shader_stage_flag_bits>(stage);
        if( !(rPass.get_stage_mask() & eStage) )
            continue;

        u32 shaderIdx;
        switch( eStage )
        {
        case SHADER_STAGE_VERTEX:
            shaderIdx = rPass.get_vertex_shader_index();
            break;
        case SHADER_STAGE_GEOMETRY:
            shaderIdx = rPass.get_geometry_shader_index();
            break;
        case SHADER_STAGE_FRAGMENT:
            shaderIdx = rPass.get_fragment_shader_index();
            break;
        default:
            GFX_ASSERT(false, "Shader stage not implemented.");
            return VK_NULL_HANDLE;
        }

        GFX_ASSERT(program->get_shader_count() > shaderIdx, "Program {} ({}) does not have shader index {}. Bad data?", program->get_name().get_hash(), program->get_name().try_get_str(), shaderIdx);
        const shader& rShader = program->get_shader(shaderIdx);

        GFX_ASSERT(rShader.get_stage() == eStage, "Shader on index {} does not match the stage.", shaderIdx);
        const dt::array<u32>& code = rShader.get_code();

        VkShaderModuleCreateInfo modCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        modCreateInfo.codeSize = code.size() * sizeof(u32);
        modCreateInfo.pCode = code.data();

        VkPipelineShaderStageCreateInfo stageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        stageCreateInfo.stage = (VkShaderStageFlagBits)converters::get_shader_stage_flags_vk(eStage);
        stageCreateInfo.pName = rShader.get_entry_point();

        VkResult modResult = vkCreateShaderModule(device, &modCreateInfo, nullptr, &stageCreateInfo.module);
        switch( modResult )
        {
        case VK_SUCCESS:
            break;
        default:
            GFX_ASSERT(false, "Failed to create shader module.");
            return VK_NULL_HANDLE;
        }

        modules.push_back(stageCreateInfo.module);
        stages.push_back(stageCreateInfo);
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineCreateInfo.stageCount = u32_cast(stages.size());
    pipelineCreateInfo.pStages = stages.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.vertexBindingDescriptionCount = rPass.get_pipeline_state().get_vertex_input_state().channel_count;

    dt::vector<VkVertexInputBindingDescription> bindings;
    dt::vector<VkVertexInputAttributeDescription> attributes;
    bindings.reserve(vertexInputInfo.vertexBindingDescriptionCount);
    for( u32 idx = 0; idx < vertexInputInfo.vertexBindingDescriptionCount; idx++ )
    {
        VkVertexInputBindingDescription binding{ };
        binding.binding = idx;
        binding.stride = rPass.get_pipeline_state().get_vertex_input_state().descriptions[idx].stride;
        binding.inputRate = converters::get_vertex_input_rate_vk(rPass.get_pipeline_state().get_vertex_input_state().descriptions[idx].input_rate);

        u32 attributeCount = rPass.get_pipeline_state().get_vertex_input_state().descriptions[idx].attribute_count;
        for( u32 dIdx = 0; dIdx < attributeCount; dIdx++ )
        {
            VkVertexInputAttributeDescription attribute{ };
            attribute.binding = idx;
            attribute.location = dIdx;
            attribute.offset = rPass.get_pipeline_state().get_vertex_input_state().descriptions[idx].attributes[dIdx].offset;
            attribute.format = converters::get_format_vk(rPass.get_pipeline_state().get_vertex_input_state().descriptions[idx].attributes[dIdx].format);

            attributes.push_back(attribute);
        }

        bindings.push_back(binding);
    }

    vertexInputInfo.vertexAttributeDescriptionCount = u32_cast(attributes.size());
    vertexInputInfo.pVertexBindingDescriptions = bindings.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssemblyInfo.topology = converters::get_topology_vk(rPass.get_pipeline_state().get_input_assembly_state().topology);
    inputAssemblyInfo.primitiveRestartEnable = rPass.get_pipeline_state().get_input_assembly_state().allow_restart ? VK_TRUE : VK_FALSE;

    VkPipelineTessellationStateCreateInfo tessellationInfo{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
    tessellationInfo.patchControlPoints = rPass.get_pipeline_state().get_tessellation_state().patch_control_points;

    VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizationInfo.depthClampEnable = rPass.get_pipeline_state().get_rasterization_state().enable_depth_clamp ? VK_TRUE : VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = rPass.get_pipeline_state().get_rasterization_state().enable_rasterizer_discard ? VK_TRUE : VK_FALSE;

    rasterizationInfo.polygonMode = converters::get_polygon_mode_vk(rPass.get_pipeline_state().get_rasterization_state().polygon_mode);
    rasterizationInfo.cullMode = converters::get_cull_mode_vk(rPass.get_pipeline_state().get_rasterization_state().cull_mode);
    rasterizationInfo.frontFace = converters::get_front_face_vk(rPass.get_pipeline_state().get_rasterization_state().front_face_mode);

    rasterizationInfo.depthBiasEnable = rPass.get_pipeline_state().get_rasterization_state().enable_depth_bias ? VK_TRUE : VK_FALSE;
    rasterizationInfo.depthBiasConstantFactor = rPass.get_pipeline_state().get_rasterization_state().depth_bias_mode.constant_factor;
    rasterizationInfo.depthBiasSlopeFactor = rPass.get_pipeline_state().get_rasterization_state().depth_bias_mode.slope_factor;

    rasterizationInfo.lineWidth = rPass.get_pipeline_state().get_rasterization_state().line_width;

    VkPipelineMultisampleStateCreateInfo multisampleInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampleInfo.rasterizationSamples = static_cast<VkSampleCountFlagBits>(converters::get_sample_count_flags_vk(rPass.get_pipeline_state().get_multisample_state().sample_count));
    multisampleInfo.sampleShadingEnable = rPass.get_pipeline_state().get_multisample_state().enable_sample_shading;
    multisampleInfo.minSampleShading = rPass.get_pipeline_state().get_multisample_state().min_sample_shading;

    VkSampleMask sampleMask = rPass.get_pipeline_state().get_multisample_state().sample_mask;
    multisampleInfo.pSampleMask = sampleMask ? &sampleMask : VK_NULL_HANDLE;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencilInfo.depthTestEnable = rPass.get_pipeline_state().get_depth_stencil_state().enable_depth_test ? VK_TRUE : VK_FALSE;
    depthStencilInfo.depthWriteEnable = rPass.get_pipeline_state().get_depth_stencil_state().write_depth ? VK_TRUE : VK_FALSE;
    depthStencilInfo.depthCompareOp = converters::get_compare_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().depth_compare);
    depthStencilInfo.depthBoundsTestEnable = rPass.get_pipeline_state().get_depth_stencil_state().enable_depth_bounds_test ? VK_TRUE : VK_FALSE;
    depthStencilInfo.stencilTestEnable = rPass.get_pipeline_state().get_depth_stencil_state().enable_stencil_test ? VK_TRUE : VK_FALSE;

    depthStencilInfo.front.failOp = converters::get_stencil_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().front_stencil.fail);
    depthStencilInfo.front.passOp = converters::get_stencil_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().front_stencil.pass);
    depthStencilInfo.front.depthFailOp = converters::get_stencil_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().front_stencil.depth_fail);
    depthStencilInfo.front.compareOp = converters::get_compare_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().front_stencil.compare);
    // TOOD?
    depthStencilInfo.front.compareMask = 0;
    depthStencilInfo.front.writeMask = 0;
    depthStencilInfo.front.reference = 0;

    depthStencilInfo.back.failOp = converters::get_stencil_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().back_stencil.fail);
    depthStencilInfo.back.passOp = converters::get_stencil_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().back_stencil.pass);
    depthStencilInfo.back.depthFailOp = converters::get_stencil_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().back_stencil.depth_fail);
    depthStencilInfo.back.compareOp = converters::get_compare_op_vk(rPass.get_pipeline_state().get_depth_stencil_state().back_stencil.compare);
    // TOOD?
    depthStencilInfo.back.compareMask = 0;
    depthStencilInfo.back.writeMask = 0;
    depthStencilInfo.back.reference = 0;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlendInfo.logicOpEnable = rPass.get_pipeline_state().get_output_blend_states().enable_blend ? VK_TRUE : VK_FALSE;
    colorBlendInfo.logicOp = converters::get_logic_op_vk(rPass.get_pipeline_state().get_output_blend_states().logic_op);
    colorBlendInfo.attachmentCount = rPass.get_pipeline_state().get_output_blend_states().state_count;
    dt::vector<VkPipelineColorBlendAttachmentState> blendStates;
    blendStates.reserve(colorBlendInfo.attachmentCount);

    for( u32 idx = 0; idx < colorBlendInfo.attachmentCount; idx++ )
    {
        VkPipelineColorBlendAttachmentState blendState{ };
        blendState.blendEnable = rPass.get_pipeline_state().get_output_blend_states().blend_states[idx].enable_blend;
        blendState.srcColorBlendFactor = converters::get_blend_factor_vk(rPass.get_pipeline_state().get_output_blend_states().blend_states[idx].src_color_factor);
        blendState.dstColorBlendFactor = converters::get_blend_factor_vk(rPass.get_pipeline_state().get_output_blend_states().blend_states[idx].dst_color_factor);
        blendState.colorBlendOp = converters::get_blend_op_vk(rPass.get_pipeline_state().get_output_blend_states().blend_states[idx].color_operation);

        blendState.srcAlphaBlendFactor = converters::get_blend_factor_vk(rPass.get_pipeline_state().get_output_blend_states().blend_states[idx].src_alpha_factor);
        blendState.dstAlphaBlendFactor = converters::get_blend_factor_vk(rPass.get_pipeline_state().get_output_blend_states().blend_states[idx].dst_alpha_factor);
        blendState.alphaBlendOp = converters::get_blend_op_vk(rPass.get_pipeline_state().get_output_blend_states().blend_states[idx].alpha_operation);

        blendState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

        blendStates.push_back(blendState);
    }

    colorBlendInfo.pAttachments = blendStates.data();

    // TODO?
    colorBlendInfo.blendConstants[0] = 1.f;
    colorBlendInfo.blendConstants[1] = 1.f;
    colorBlendInfo.blendConstants[2] = 1.f;
    colorBlendInfo.blendConstants[3] = 1.f;

    dt::inline_array<VkDynamicState, 3> dynamicStates;
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
    dynamicStates[2] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;

    VkPipelineDynamicStateCreateInfo dynamicInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicInfo.dynamicStateCount = u32_cast(dynamicStates.size());
    dynamicInfo.pDynamicStates = dynamicStates.data();

    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pTessellationState = &tessellationInfo;
    pipelineCreateInfo.pViewportState = &viewportInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    pipelineCreateInfo.pDynamicState = &dynamicInfo;
    
    pipelineCreateInfo.layout = rPass.get_layout_impl<VkPipelineLayout>();

    VkPipelineRenderingCreateInfo renderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = rPass.get_outputs().color_output_count;
    renderingInfo.depthAttachmentFormat = converters::get_format_vk(rPass.get_outputs().depth_output);
    renderingInfo.stencilAttachmentFormat = converters::get_format_vk(rPass.get_outputs().stencil_output);

    dt::vector<VkFormat> outputColorFormats;
    outputColorFormats.reserve(renderingInfo.colorAttachmentCount);
    for( u32 idx = 0; idx < renderingInfo.colorAttachmentCount; idx++ )
    {
        outputColorFormats.push_back(converters::get_format_vk(rPass.get_outputs().color_outputs[idx]));
    }

    renderingInfo.pColorAttachmentFormats = outputColorFormats.data();

    pipelineCreateInfo.pNext = &renderingInfo;
    pipelineCreateInfo.subpass = 0; // ?

    VkPipeline retval{ VK_NULL_HANDLE };
    VkResult pipelineResult = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &retval);
    switch( pipelineResult )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create Graphics Pipeline.");
        break; // Still need to destroy the modules.
    }

    // ShaderModules can just be destroyed after we've made the pipeline.
    for( VkShaderModule module : modules )
    {
        vkDestroyShaderModule(device, module, nullptr);
    }

    return retval;
}

void* VK_DEVICE::create_shader_pass_impl(program* program, u64 passIdx)
{
    return vulkan_create_graphics_pipeline_impl(get_impl<device_state_vk>().device, program, passIdx);
}

void* VK_DEVICE::create_shader_pass_layout_impl(pass* pass)
{
    dt::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve(DESCRIPTOR_TABLE_COUNT);
    for( u64 i = 0; i < pass->get_descriptor_table_count(); i++ )
    {
        layouts.push_back(pass->get_descriptor_table((descriptor_table_type)i)->get_impl<VkDescriptorSetLayout>());
    }

    VkPipelineLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    createInfo.setLayoutCount = u32_cast(layouts.size());
    createInfo.pSetLayouts = layouts.data();

    VkPipelineLayout retval{ VK_NULL_HANDLE };
    VkResult result = vkCreatePipelineLayout(get_impl<device_state_vk>().device, &createInfo, nullptr, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create VkPipelineLayout.");
    }

    return retval;
}

void* VK_DEVICE::create_descriptor_table_desc_impl(descriptor_table_desc* desc)
{
    // Implicitly ordered buffer descriptors -> image descriptors
    const dt::array<descriptor_slot_desc>& bufferSlots = desc->get_buffer_descriptions();
    const dt::array<descriptor_slot_desc>& imageSlots = desc->get_image_descriptions();

    dt::vector<VkDescriptorSetLayoutBinding> bindings(bufferSlots.size() + imageSlots.size());
    u64 curIndex = 0;

    // TODO: could probably combine these into one array for convinience but this'll do for now.
    for( const auto& slot : bufferSlots )
    {
        VkDescriptorSetLayoutBinding binding{ };
        binding.binding = u32_cast(curIndex++);
        binding.stageFlags = converters::get_shader_stage_flags_vk(slot.get_visibility());
        binding.descriptorCount = slot.get_array_size();
        binding.descriptorType = converters::get_descriptor_type_vk(slot.get_resource_type());

        bindings.push_back(binding);
    }

    for( const auto& slot : imageSlots )
    {
        VkDescriptorSetLayoutBinding binding{ };
        binding.binding = u32_cast(curIndex++);
        binding.stageFlags = converters::get_shader_stage_flags_vk(slot.get_visibility());
        binding.descriptorCount = slot.get_array_size();
        binding.descriptorType = converters::get_descriptor_type_vk(slot.get_resource_type());

        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    createInfo.bindingCount = u32_cast(bindings.size());
    createInfo.pBindings = bindings.data();

    VkDescriptorSetLayout retval{ VK_NULL_HANDLE };
    VkResult result = vkCreateDescriptorSetLayout(get_impl<device_state_vk>().device, &createInfo, nullptr, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create DescriptorSetLayout.");
    }
    return retval;
}

void VK_DEVICE::destroy_descriptor_table_desc(descriptor_table_desc* desc)
{
    vkDestroyDescriptorSetLayout(get_impl<device_state_vk>().device, desc->get_impl<VkDescriptorSetLayout>(), nullptr);
}

void VK_DEVICE::destroy_shader_program(program* program)
{
    device_state_vk& deviceState = get_impl<device_state_vk>();

    for( u64 passIdx = 0; passIdx < program->get_pass_count(); passIdx++ )
    {
        const pass& pass = program->get_pass(passIdx);

        // These two doesn't really matter what order
        vkDestroyPipelineLayout(deviceState.device, pass.get_layout_impl<VkPipelineLayout>(), nullptr);
        vkDestroyPipeline(deviceState.device, pass.get_impl<VkPipeline>(), nullptr);
    }
}

void* VK_DEVICE::create_descriptor_pool_impl(descriptor_table_desc* base, u32 size, bool reuse_tables)
{
    dt::vector<VkDescriptorPoolSize> sizes;
    // Just do this slow as fuck who cares
    for( const auto& slot : base->get_buffer_descriptions() )
    {
        bool found = false;
        VkDescriptorType vkType = converters::get_descriptor_type_vk(slot.get_resource_type());
        // Find if we already have one of this descriptor type
        for( VkDescriptorPoolSize& pool_size : sizes )
        {
            if( pool_size.type == vkType )
            {
                pool_size.descriptorCount++;
                found = true;
                break;
            }
        }

        if( !found )
        {
            VkDescriptorPoolSize insert{ };
            insert.type = vkType;
            insert.descriptorCount = 1;
            sizes.push_back(insert);
        }
    }

    for( const auto& slot : base->get_image_descriptions() )
    {
        bool found = false;
        VkDescriptorType vkType = converters::get_descriptor_type_vk(slot.get_resource_type());
        // Find if we already have one of this descriptor type
        for( VkDescriptorPoolSize& pool_size : sizes )
        {
            if( pool_size.type == vkType )
            {
                pool_size.descriptorCount++; // Does this need to be size?
                found = true;
                break;
            }
        }

        if( !found )
        {
            VkDescriptorPoolSize insert{ };
            insert.type = vkType;
            insert.descriptorCount = 1;
            sizes.push_back(insert);
        }
    }

    VkDescriptorPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    createInfo.maxSets = size;
    createInfo.poolSizeCount = u32_cast(sizes.size());
    createInfo.pPoolSizes = sizes.data();
    createInfo.flags = reuse_tables ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0;

    VkDescriptorPool retval{ VK_NULL_HANDLE };
    VkResult result = vkCreateDescriptorPool(get_impl<device_state_vk>().device, &createInfo, nullptr, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create descriptor pool.");
        return VK_NULL_HANDLE;
    }

    return retval;
}

void VK_DEVICE::destroy_descriptor_pool(descriptor_pool* pool)
{
    vkDestroyDescriptorPool(get_impl<device_state_vk>().device, pool->get_impl<VkDescriptorPool>(), nullptr);
}

void VK_DEVICE::reset_descriptor_pool(descriptor_pool* pool)
{
    vkResetDescriptorPool(get_impl<device_state_vk>().device, pool->get_impl<VkDescriptorPool>(), 0);
}

void* VK_DEVICE::allocate_descriptor_table_impl(descriptor_pool* pool)
{
    VkDescriptorPool vkPool = pool->get_impl<VkDescriptorPool>();
    VkDescriptorSetLayout setLayout = pool->get_table_desc().get_impl<VkDescriptorSetLayout>();

    VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = vkPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &setLayout;

    VkDescriptorSet retval{ VK_NULL_HANDLE };
    VkResult result = vkAllocateDescriptorSets(get_impl<device_state_vk>().device, &allocInfo, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to allocate descriptor set.");
        return VK_NULL_HANDLE;
    }
    return retval;
}

void VK_DEVICE::write_descriptor_table(descriptor_table* table)
{
    const std::vector<buffer*>& bufferViews = table->get_buffer_views();
    const std::vector<void*>& imageViews = table->get_image_views();

    u32 bindingIdx = 0;
    if( bufferViews.size() != 0 )
    {
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        for( buffer* buffer : bufferViews )
        {
            VkDescriptorBufferInfo info{ };
            info.buffer = buffer->get_impl<VkBuffer>();
            info.offset = 0;
            info.range = VK_WHOLE_SIZE;

            bufferInfos.push_back(info);
        }

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.dstSet = table->get_impl<VkDescriptorSet>();
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = u32_cast(bufferInfos.size());
        write.pBufferInfo = bufferInfos.data();

        vkUpdateDescriptorSets(get_impl<device_state_vk>().device, 1, &write, 0, nullptr);
    }

    if( imageViews.size() != 0 )
    {
        std::vector<VkDescriptorImageInfo> imageInfos;
        for( void* pSampler : imageViews )
        {
            // TODO assume sampler, lol.
            texture_sampler* sampler = reinterpret_cast<texture_sampler*>(pSampler);

            VkDescriptorImageInfo info{ };
            info.imageLayout = converters::get_layout_vk(sampler->get_texture_view()->get_resource()->get_layout());
            info.imageView = sampler->get_texture_view()->get_impl<VkImageView>();
            info.sampler = sampler->get_impl<VkSampler>();

            imageInfos.push_back(info);
        }

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.dstSet = table->get_impl<VkDescriptorSet>();
        write.dstBinding = u32_cast(bufferViews.size());
        write.dstArrayElement = 0;
        write.descriptorCount = u32_cast(imageInfos.size());
        write.pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(get_impl<device_state_vk>().device, 1, &write, 0, nullptr);
    }
}

VkQueue device_state_vk::get_queue(u32 familyIdx, u32 idx) const
{
    GFX_ASSERT(u32_cast(queue_families.size()) > familyIdx, "Family index is invalid.");

    const queue_family& family = queue_families[familyIdx];
    GFX_ASSERT(u32_cast(family.queues.size()) > idx, "Queue index is invalid.");

    return family.queues[idx];
}

VkQueue device_state_vk::get_queue_by_flags(VkQueueFlags flags, u32 idx) const
{
    for( const queue_family& family : queue_families )
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

VkQueue device_state_vk::get_queue_by_present(u32 idx) const
{
    for( const queue_family& family : queue_families )
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

u32 device_state_vk::get_queue_family_count() const
{
    return u32_cast(queue_families.size());
}

u32 device_state_vk::get_family_index_by_flags(VkQueueFlags flags) const
{
    for( u64 i = 0; i < queue_families.size(); i++ )
    {
        const queue_family& family = queue_families[i];
        if( (family.properties.queueFlags & flags) == flags )
        {
            return u32_cast(i);
        }
    }

    return u32_max;
}

bool device_state_vk::queue_family_supports_present(u32 familyIdx) const
{
    GFX_ASSERT(u32_cast(queue_families.size()) < familyIdx, "Family index is invalid.");
    return queue_families[familyIdx].flags & queue_family_flag_bit::supports_present;
}

void device_state_vk::create_instance(debugger& debugger)
{
    debugger.attach_callback([](debugger::severity level, const char* message)
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
                    break;
                default:
                    GFX_INFO(message);
                    break;
            }
        });

    // Instance layers.
    u32 layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    available_instance_layers.resize(layerCount);

    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    for( u64 i = 0; i < layers.size(); i++ )
    {
        available_instance_layers[i] = layers[i].layerName;
    }

    std::vector<const char*> enabledLayers;
    for( const char* reqLayer : vulkan_get_instance_layers() )
    {
        bool enabled = false;
        for( std::string& lyr : available_instance_layers )
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
    available_instance_extensions.resize(extCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extensions.data());

    for( u64 i = 0; i < extensions.size(); i++ )
    {
        available_instance_extensions[i] = extensions[i].extensionName;
    }

    std::vector<const char*> enableExtensions;
    for( const char* reqExt : vulkan_get_instance_extensions() )
    {
        bool enabled = false;
        for( std::string& ext : available_instance_extensions )
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
    dbgCreateInfo.pUserData = &debugger;

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
    createInfo.pNext = debugger.is_enabled() ? &dbgCreateInfo : nullptr;

    // Validation layers.

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    switch( result )
    {
        case VK_SUCCESS:
            break;
        default:
            break;
    }

    enabled_instance_extensions = std::move(enableExtensions);
    enabled_instance_layers = std::move(enabledLayers);

    if( debugger.is_enabled() )
    {
        VkDebugUtilsMessengerEXT dbgMessenger{ };
        VkResult dbgResult = create_debug_utils_messenger(instance, &dbgCreateInfo, nullptr, &dbgMessenger);
        switch( dbgResult )
        {
        case VK_SUCCESS:
            break;
        default:
            break;
        }

        debugger.set_impl_ptr(dbgMessenger);
    }
}

void VK_DEVICE::dump_info() const
{
    const device_state_vk& deviceState = get_impl<device_state_vk>();

    GFX_VERBOSE("Instance extensions:");
    for( const char* ext : deviceState.enabled_instance_extensions )
    {
        GFX_VERBOSE("\t*{}", ext);
    }
    for( const std::string& ext : deviceState.available_instance_extensions )
    {
        bool output = true;
        for( const char* ext2 : deviceState.enabled_instance_extensions )
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
    for( const char* lyr : deviceState.enabled_instance_layers )
    {
        GFX_VERBOSE("\t*{}", lyr);
    }
    for( const std::string& lyr : deviceState.available_instance_layers )
    {
        bool output = true;
        for( const char* lyr2 : deviceState.enabled_instance_layers )
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
    for( const char* ext : deviceState.enabled_device_extensions )
    {
        GFX_VERBOSE("\t*{}", ext);
    }
    for( const std::string& ext : deviceState.available_device_extensions )
    {
        bool output = true;
        for( const char* ext2 : deviceState.enabled_device_extensions )
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
#endif // GFX_SUPPORTS_VULKAN