#include "device_vk.h"

#ifdef GFX_SUPPORTS_VULKAN
#include "vkconverts.h"

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

    VkPhysicalDevice physicalDevice = m_gpu.get_impl<VkPhysicalDevice>();
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

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    // Create device
    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.pNext = &dynamicRenderingFeatures;
    createInfo.queueCreateInfoCount = u32_cast(queueInfos.size());
    createInfo.pQueueCreateInfos = queueInfos.data();
    createInfo.enabledExtensionCount = u32_cast(m_enabledDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_enabledDeviceExtensions.data();

    VkPhysicalDeviceFeatures features{ };
    features.wideLines = true;
    features.fillModeNonSolid = true;
    createInfo.pEnabledFeatures = &features; // TODO

    VkResult deviceResult = vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_device);
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
    for( queue_family& family : m_queueFamilies )
    {
        for( u32 idx = 0; idx < u32_cast(family.queues.size()); idx++ )
        {
            vkGetDeviceQueue(m_device, family.index, idx, &family.queues[idx]);
        }
    }

    // Create allocator
#ifdef GFX_VK_VMA_ALLOCATOR
    m_allocator.initialise(m_instance, m_gpu.get_impl<VkPhysicalDevice>(), m_device);
#endif

    // Create command pools
    m_commandPool.initialise();

    return 0;
}

u32 device_vk::initialise(u32 gpuIdx)
{
    return initialise(gpuIdx, nullptr);
}

void device_vk::shutdown()
{
    m_allocator.shutdown();
    m_commandPool.shutdown();

    if( m_surface )
    {
        vkDestroySurfaceKHR(m_instance, (VkSurfaceKHR)m_surface, nullptr);
    }

    vkDestroyDevice(m_device, nullptr);

    return;
}

#ifdef GFX_EXT_SWAPCHAIN
surface_capabilities device_vk::get_surface_capabilities() const
{
    GFX_ASSERT(m_surface, "Surface was not provided when creating graphics device.");

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

swapchain device_vk::create_swapchain(swapchain* previous, texture_info info, present_mode present)
{
    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.oldSwapchain = previous
        ? previous->get_impl<VkSwapchainKHR>()
        : VK_NULL_HANDLE;
    createInfo.minImageCount = 2;
    createInfo.imageExtent = { info.get_width(), info.get_height() };

    // TODO
    createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageFormat = converters::get_format_cdt_vk(info.get_format());
    createInfo.imageArrayLayers = info.get_depth();
    createInfo.imageUsage = info.get_usage();

    //TODO
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    
    createInfo.presentMode = (VkPresentModeKHR)present;
    createInfo.surface = (VkSurfaceKHR)m_surface;
    
    VkSwapchainKHR retSwapchain{ VK_NULL_HANDLE };

    VkResult result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &retSwapchain);
    GFX_ASSERT(result != VK_ERROR_INITIALIZATION_FAILED, "Failed to create graphics swapchain. Ensure that '{}' device extension is enabled.", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    GFX_ASSERT(result == VK_SUCCESS, "Failed to create graphics swapchain.");

    u32 imageCount{ 0 };
    vkGetSwapchainImagesKHR(m_device, retSwapchain, &imageCount, nullptr);

    std::vector<VkImage> images;
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, retSwapchain, &imageCount, images.data());

    std::vector<texture> textures;
    textures.reserve(imageCount);

    for( VkImage image : images )
    {
        // Make a full view for now?
        VkImageView pViewImpl{ VK_NULL_HANDLE };

        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = image;
        viewInfo.format = converters::get_format_cdt_vk(info.get_format());
        viewInfo.viewType = converters::get_view_type_vk(resource_view_type::texture_2d);

        VkImageSubresourceRange range{ };
        if( cdt::is_depth_format(info.get_format()) )
            range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        else
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.baseArrayLayer = 0;
        range.levelCount = 1;
        range.layerCount = 1;

        viewInfo.subresourceRange = range;

        VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &pViewImpl);
        switch( result )
        {
        case VK_SUCCESS:
            break;
        default:
            GFX_ASSERT(false, "Image view creation failed.");
        }

        textures.emplace_back();

        memory_info blankInfo{ };
        textures.back().initialise(blankInfo, info, image, pViewImpl, true);
    }

    swapchain retval;
    retval.initialise(std::move(textures), retSwapchain);
    return retval;
}

void device_vk::free_swapchain(swapchain* swapchain)
{
    u32 imageCount = swapchain->get_image_count();
    for( u32 idx = 0; idx < imageCount; idx++ )
    {
        free_texture(swapchain->get_image(idx));
        free_fence(swapchain->get_fence(idx));
    }

    vkDestroySwapchainKHR(m_device, swapchain->get_impl<VkSwapchainKHR>(), nullptr);
}
#endif // GFX_EXT_SWAPCHAIN

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

        out.push_back({ });
        out.back().initialise(properties.deviceName,
                              i,
                              totalSize,
                              properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                              device);
    }
    return out;
}

buffer device_vk::create_buffer(u64 size, buffer_usage usage, memory_type mem_type, bool persistant)
{
    memory_info memoryInfo{ };
    memoryInfo.size = size;
    memoryInfo.type = u32_cast(mem_type);
    memoryInfo.persistant = u32_cast(persistant);
    memoryInfo.viewType = u32_cast(resource_view_type::buffer);

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

    buffer retval;
    retval.initialise(memoryInfo, usage, pImpl);
    return retval;
}

void device_vk::free_buffer(buffer* buf)
{
    GFX_ASSERT(buf->is_persistant() || !buf->is_mapped(), "Buffer should be unmapped before freeing.");

    if( buf->is_persistant() )
    {
    #ifdef GFX_VK_VMA_ALLOCATOR
        m_allocator.unmap(buf->get_backing_memory<VmaAllocation>());
    #endif
    }

#ifdef GFX_VK_VMA_ALLOCATOR
    m_allocator.free_buffer({ buf->get_impl<VkBuffer>(), buf->get_backing_memory<VmaAllocation>() });
#endif
}

texture device_vk::create_texture(texture_info info, resource_view_type view_type, memory_type mem_type, bool persistant)
{
    memory_info memoryInfo{ };
    memoryInfo.size = info.get_size();
    memoryInfo.type = u32_cast(mem_type);
    memoryInfo.persistant = u32_cast(persistant);
    memoryInfo.viewType = u32_cast(view_type);

    void* pImpl = nullptr;
#ifdef GFX_VK_VMA_ALLOCATOR
    vma_allocation<VkImage> allocation = m_allocator.allocate_image(info, view_type, mem_type);
    memoryInfo.backing_memory = allocation.allocation;
    pImpl = allocation.resource;
#endif

    if( persistant )
    {
    #ifdef GFX_VK_VMA_ALLOCATOR
        memoryInfo.mapped = m_allocator.map((VmaAllocation)memoryInfo.backing_memory);
    #endif
    }

    // Make a full view for now?
    VkImageView pViewImpl{ VK_NULL_HANDLE };

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = static_cast<VkImage>(pImpl);
    viewInfo.format = converters::get_format_cdt_vk(info.get_format());
    viewInfo.viewType = converters::get_view_type_vk(view_type);
    
    VkImageSubresourceRange range{ };
    if( cdt::is_depth_format(info.get_format()) )
        range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.baseArrayLayer = 0;
    range.levelCount = 1;
    range.layerCount = 1;

    viewInfo.subresourceRange = range;

    VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &pViewImpl);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Image view creation failed.");
    }

    texture retval;
    retval.initialise(memoryInfo, info, pImpl, pViewImpl);
    return retval;
}

void device_vk::free_texture(texture* tex)
{
    GFX_ASSERT(tex->is_persistant() || !tex->is_mapped(), "Image should be unmapped before freeing.");

    if( tex->is_persistant() )
    {
    #ifdef GFX_VK_VMA_ALLOCATOR
        m_allocator.unmap(tex->get_backing_memory<VmaAllocation>());
    #endif
    }

    vkDestroyImageView(m_device, tex->get_view_impl<VkImageView>(), nullptr);

#ifdef GFX_VK_VMA_ALLOCATOR
    if( !tex->is_swapchain_image() )
    {
        m_allocator.free_image({ tex->get_impl<VkImage>(), tex->get_backing_memory<VmaAllocation>() });
    }
#endif
}

fence device_vk::create_fence(bool signaled)
{
    VkFence impl{ VK_NULL_HANDLE };
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    VkResult result = vkCreateFence(m_device, &createInfo, nullptr, &impl);

    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Fence creation failed.");
    }

    return fence(impl);
}

void device_vk::free_fence(fence* fence)
{
    vkDestroyFence(m_device, fence->get_impl<VkFence>(), nullptr);
}

dependency device_vk::create_dependency(const char* debug_name)
{
    dependency retval{ };
    VkSemaphore retSemaphore{ VK_NULL_HANDLE };

    VkSemaphoreCreateInfo createInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkResult result = vkCreateSemaphore(m_device, &createInfo, nullptr, &retSemaphore);
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

void device_vk::free_dependency(dependency* dep)
{
    vkDestroySemaphore(m_device, dep->get_impl<VkSemaphore>(), nullptr);
}

graphics_command_list device_vk::allocate_graphics_command_list()
{
    VkCommandBuffer buffer = m_commandPool.allocate_buffer_by_flags(VK_QUEUE_GRAPHICS_BIT);
    graphics_command_list retval{ };
    retval.init(buffer);
    return retval;
}

void device_vk::free_command_list(command_list* list)
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

    m_commandPool.free_buffer_by_flags(list->get_impl<VkCommandBuffer>(), flags);
}

void device_vk::map(buffer* buf)
{
    map_impl(&buf->get_memory_info());
}

void device_vk::map(texture* tex)
{
    map_impl(&tex->get_memory_info());
}

void device_vk::unmap(buffer* buf)
{
    unmap_impl(&buf->get_memory_info());
}

void device_vk::unmap(texture* tex)
{
    unmap_impl(&tex->get_memory_info());
}

void device_vk::map_impl(memory_info* memInfo)
{
    GFX_ASSERT(!memInfo->persistant, "Persistant memory cannot be manually mapped.");
    GFX_ASSERT(static_cast<memory_type>(memInfo->type) == memory_type::cpu_accessible, "Cannot map non-cpu accessible memory.");
    
#ifdef GFX_VK_VMA_ALLOCATOR
    memInfo->mapped = m_allocator.map((VmaAllocation)memInfo->backing_memory);
#endif
}

void device_vk::unmap_impl(memory_info* memInfo)
{
    GFX_ASSERT(!memInfo->persistant, "Persistant memory cannot be manually unmapped.");
    GFX_ASSERT(!memInfo->mapped, "Buffer that isn't mapped cannot be unmapped.");

#ifdef GFX_VK_VMA_ALLOCATOR
    m_allocator.unmap((VmaAllocation)memInfo->backing_memory);
#endif
}

void device_vk::wait_idle()
{
    vkDeviceWaitIdle(m_device);
}

#ifdef GFX_EXT_SWAPCHAIN
u32 device_vk::acquire_next_image(swapchain* swapchain, fence* fence, u64 timeout)
{
    VkFence signalFence{ VK_NULL_HANDLE };
    if( fence )
    {
        signalFence = fence->get_impl<VkFence>();
    }

    u32 retval{ 0 };
    VkResult result = vkAcquireNextImageKHR(m_device, swapchain->get_impl<VkSwapchainKHR>(), timeout, VK_NULL_HANDLE, signalFence, &retval);

    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to acquire next swapchain image.");
    }

    return retval;
}

void device_vk::present(swapchain* swapchain, u32 image_index, const std::vector<dependency*>& dependencies)
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

    vkQueuePresentKHR(get_queue_by_present(), &presentInfo);
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

bool device_vk::wait_for_fences(const fence* pFences, u32 count, bool wait_for_all, u64 timeout) const
{
    VkFence* fences = new VkFence[count];
    for( u32 i = 0; i < count; i++ )
    {
        fences[i] = pFences[i].get_impl<VkFence>();
    }

    VkResult result = vkWaitForFences(m_device, count, fences, wait_for_all, timeout);
    delete[] fences;

    return result == VK_SUCCESS;
}

bool device_vk::reset_fences(fence* pFences, u32 count)
{
    VkFence* fences = new VkFence[count];
    for( u32 i = 0; i < count; i++ )
    {
        fences[i] = pFences[i].get_impl<VkFence>();
    }

    VkResult result = vkResetFences(m_device, count, fences);
    delete[] fences;

    return result == VK_SUCCESS;
}

bool device_vk::check_fence(const fence* fence) const
{
    VkResult result = vkGetFenceStatus(m_device, fence->get_impl<VkFence>());
    return result == VK_SUCCESS;
}

void device_vk::reset(command_list* list)
{
    VkCommandBufferResetFlags flags{ };
    // flags |= VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;

    vkResetCommandBuffer(list->get_impl<VkCommandBuffer>(), flags);
}

void device_vk::begin(command_list* list)
{
    VkCommandBufferBeginInfo begin{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    // FLAGS?

    vkBeginCommandBuffer(list->get_impl<VkCommandBuffer>(), &begin);
}

void device_vk::end(command_list* list)
{
    vkEndCommandBuffer(list->get_impl<VkCommandBuffer>());
}

void device_vk::submit(const std::vector<graphics_command_list*>& lists, fence* fence)
{
    VkQueue queue = get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT);
    std::vector<command_list*> castLists;
    castLists.reserve(lists.size());

    for( graphics_command_list* list : lists )
    {
        castLists.push_back(reinterpret_cast<command_list*>(list));
    }

    submit_impl(queue, castLists, fence);
}

void device_vk::submit(const std::vector<compute_command_list*>& lists, fence* fence)
{
    VkQueue queue = get_queue_by_flags(VK_QUEUE_COMPUTE_BIT);
    std::vector<command_list*> castLists;
    castLists.reserve(lists.size());

    for( compute_command_list* list : lists )
    {
        castLists.push_back(reinterpret_cast<command_list*>(list));
    }

    submit_impl(queue, castLists, fence);
}

void device_vk::submit(const std::vector<present_command_list*>& lists, fence* fence)
{
    VkQueue queue = get_queue_by_present();
    std::vector<command_list*> castLists;
    castLists.reserve(lists.size());

    for( present_command_list* list : lists )
    {
        castLists.push_back(reinterpret_cast<command_list*>(list));
    }

    submit_impl(queue, castLists, fence);
}

void device_vk::submit_impl(VkQueue queue, const std::vector<command_list*>& lists, fence* fence)
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

    for( const command_list* list : lists )
    {
        for( const dependency* dep : list->get_wait_dependencies() )
        {
            waitSema.push_back(dep->get_impl<VkSemaphore>());
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

    VkFence submitFence = fence
        ? fence->get_impl<VkFence>()
        : VK_NULL_HANDLE;
    vkQueueSubmit(queue, 1, &info, submitFence);
}

void device_vk::draw(command_list* list, u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
{
    vkCmdDraw(list->get_impl<VkCommandBuffer>(), vertex_count, instance_count, first_vertex, first_instance);
}

void device_vk::draw_indexed(command_list* list, u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance)
{
    vkCmdDrawIndexed(list->get_impl<VkCommandBuffer>(), index_count, instance_count, first_index, vertex_offset, first_instance);
}

void device_vk::bind_vertex_buffers(command_list* list, const std::vector<buffer*>& buffers, u32 first_vertex_index)
{
    std::vector<VkBuffer> bufs;
    std::vector<VkDeviceSize> offsets;

    for( u64 i = 0; i < buffers.size(); i++ )
    {
        bufs.push_back(buffers[i]->get_impl<VkBuffer>());
        // TODO offset
        offsets.push_back(0);
    }

    vkCmdBindVertexBuffers(list->get_impl<VkCommandBuffer>(), first_vertex_index, u32_cast(buffers.size()), bufs.data(), offsets.data());
}

void device_vk::bind_index_buffer(command_list* list, buffer* buffer, index_buffer_type index_type)
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

void device_vk::begin_pass(command_list* list, program* program, u64 passIdx, texture* output)
{
    VkRenderingInfo renderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    
    VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    attachmentInfo.imageView = output->get_view_impl<VkImageView>();
    attachmentInfo.imageLayout = converters::get_layout_vk(output->get_layout());
    attachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &attachmentInfo;
    renderInfo.renderArea = { 0, 0, 1430, 1079 };
    renderInfo.layerCount = 1;

    vkCmdBeginRendering(list->get_impl<VkCommandBuffer>(), &renderInfo);
    vkCmdBindPipeline(list->get_impl<VkCommandBuffer>(), VK_PIPELINE_BIND_POINT_GRAPHICS, program->get_pass(passIdx).get_impl<VkPipeline>());

    VkViewport viewport{ };
    viewport.x = 0.f;
    viewport.y = f32_cast(1079);
    viewport.width = f32_cast(1430);
    viewport.height = -f32_cast(1079);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{ };
    scissor.offset = { 0, 0 };
    scissor.extent = { 1430, 1079 };

    vkCmdSetViewport(list->get_impl<VkCommandBuffer>(), 0, 1, &viewport);
    vkCmdSetScissor(list->get_impl<VkCommandBuffer>(), 0, 1, &scissor);
}

void device_vk::end_pass(command_list* list)
{
    vkCmdEndRendering(list->get_impl<VkCommandBuffer>());
}

void device_vk::copy_texture_to_texture(command_list* list, texture* src, texture* dst)
{
    VkImageSubresourceLayers srcSubresource{ };
    srcSubresource.aspectMask = cdt::is_depth_format(src->get_format())
        ? VK_IMAGE_ASPECT_DEPTH_BIT
        : VK_IMAGE_ASPECT_COLOR_BIT;
    srcSubresource.mipLevel = 0;
    srcSubresource.baseArrayLayer = 0;
    srcSubresource.layerCount = 1;

    VkImageSubresourceLayers dstSubresource{ };
    dstSubresource.aspectMask = cdt::is_depth_format(dst->get_format())
        ? VK_IMAGE_ASPECT_DEPTH_BIT
        : VK_IMAGE_ASPECT_COLOR_BIT;
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

void device_vk::copy_buffer_to_texture(command_list* list, buffer* src, texture* dst)
{
    VkImageSubresourceLayers dstSubresource{ };
    dstSubresource.aspectMask = cdt::is_depth_format(dst->get_format())
        ? VK_IMAGE_ASPECT_DEPTH_BIT
        : VK_IMAGE_ASPECT_COLOR_BIT;
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

void device_vk::copy_buffer_to_buffer(command_list* list, buffer* src, buffer* dst)
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

void device_vk::texture_barrier(command_list* list, texture* texture, texture_layout dst_layout)
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

    // Pipeline stage? Comes into synchronization problem.
    vkCmdPipelineBarrier(
        list->get_impl<VkCommandBuffer>(),
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);
}

VkPipeline device_vk::create_graphics_pipeline_impl(program* program, u64 passIdx)
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

        VkResult modResult = vkCreateShaderModule(m_device, &modCreateInfo, nullptr, &stageCreateInfo.module);
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
    VkResult pipelineResult = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &retval);
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
        vkDestroyShaderModule(m_device, module, nullptr);
    }

    return retval;
}

void* device_vk::create_shader_pass_impl(program* program, u64 passIdx)
{
    return create_graphics_pipeline_impl(program, passIdx);
}

void* device_vk::create_shader_pass_layout_impl(pass* pass)
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
    VkResult result = vkCreatePipelineLayout(m_device, &createInfo, nullptr, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create VkPipelineLayout.");
    }

    return retval;
}

void* device_vk::create_descriptor_table_desc_impl(descriptor_table_desc* desc)
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
    VkResult result = vkCreateDescriptorSetLayout(m_device, &createInfo, nullptr, &retval);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create DescriptorSetLayout.");
    }
    return retval;
}

void device_vk::destroy_descriptor_table_desc(descriptor_table_desc* desc)
{
    vkDestroyDescriptorSetLayout(m_device, desc->get_impl<VkDescriptorSetLayout>(), nullptr);
}

void device_vk::destroy_shader_program(program* program)
{
    for( u64 passIdx = 0; passIdx < program->get_pass_count(); passIdx++ )
    {
        const pass& pass = program->get_pass(passIdx);

        // Destroy the tables first.
        for( u64 tableIdx = 0; tableIdx < pass.get_descriptor_table_count(); tableIdx++ )
        {
            const descriptor_table_desc* table = pass.get_descriptor_table((descriptor_table_type)tableIdx);
            vkDestroyDescriptorSetLayout(m_device, table->get_impl<VkDescriptorSetLayout>(), nullptr);
        }

        // These two doesn't really matter what order
        vkDestroyPipelineLayout(m_device, pass.get_layout_impl<VkPipelineLayout>(), nullptr);
        vkDestroyPipeline(m_device, pass.get_impl<VkPipeline>(), nullptr);
    }
}

void* device_vk::create_descriptor_pool_impl(descriptor_table_desc* base, u32 size)
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

    VkDescriptorPool retval{ VK_NULL_HANDLE };
    VkResult result = vkCreateDescriptorPool(m_device, &createInfo, nullptr, &retval);
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

void device_vk::destroy_descriptor_pool(descriptor_pool* pool)
{
    vkDestroyDescriptorPool(m_device, pool->get_impl<VkDescriptorPool>(), nullptr);
}

void device_vk::reset_descriptor_pool(descriptor_pool* pool)
{
    vkResetDescriptorPool(m_device, pool->get_impl<VkDescriptorPool>(), 0);
}

void* device_vk::allocate_descriptor_table_impl(descriptor_pool* pool)
{
    VkDescriptorPool vkPool = pool->get_impl<VkDescriptorPool>();
    VkDescriptorSetLayout setLayout = pool->get_table_desc().get_impl<VkDescriptorSetLayout>();

    VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = vkPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &setLayout;

    VkDescriptorSet retval{ VK_NULL_HANDLE };
    VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, &retval);
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

void device_vk::write_descriptor_table(descriptor_table* table)
{
    const dt::array<void*>& bufferViews = table->get_buffer_views();
    const dt::array<void*>& imageViews = table->get_image_views();

    u32 bindingIdx = 0;
    if( bufferViews.size() != 0 )
    {

    }

    if( imageViews.size() != 0 )
    {

    }
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

u32 device_vk::get_queue_family_count() const
{
    return u32_cast(m_queueFamilies.size());
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

VkDevice device_vk::get_impl_device() const
{
    return m_device;
}

VkInstance device_vk::get_impl_instance() const
{
    return m_instance;
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
    return { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_surface", "VK_KHR_win32_surface" };
}

std::vector<const char*> device_vk::get_instance_layers() const
{
    return { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };
}

std::vector<const char*> device_vk::get_device_extensions() const
{
    return { "VK_KHR_swapchain", "VK_KHR_dynamic_rendering" };
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
#endif // GFX_SUPPORTS_VULKAN