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
    createInfo.imageFormat = converters::get_format_vk(info.get_format());
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
        textures.emplace_back();

        memory_info blankInfo{ };
        textures.back().initialise(blankInfo, info, image, nullptr);
    }

    swapchain retval;
    retval.initialise(std::move(textures), retSwapchain);
    return retval;
}

void device_vk::free_swapchain(swapchain* swapchain)
{
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
    viewInfo.format = converters::get_format_vk(info.get_format());
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
    m_allocator.free_image({ tex->get_impl<VkImage>(), tex->get_backing_memory<VmaAllocation>() });
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

    if( !dependencies.empty() )
    {
        std::vector<VkSemaphore> waitSema;
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

void device_vk::bind_vertex_buffers(command_list* list, buffer* pBuffers, u32 buffer_count, u32 first_vertex_index)
{
    VkBuffer* buffers = new VkBuffer[buffer_count];
    VkDeviceSize* offsets = new VkDeviceSize[buffer_count];

    for( u32 i = 0; i < buffer_count; i++ )
    {
        buffers[i] = pBuffers->get_impl<VkBuffer>();
        // TODO offset
        offsets[i] = 0;
    }

    vkCmdBindVertexBuffers(list->get_impl<VkCommandBuffer>(), first_vertex_index, buffer_count, buffers, offsets);
    delete[] buffers;
    delete[] offsets;
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

void* device_vk::create_shader_pass_impl(program* program, u64 pass)
{
    return nullptr;
}

void* device_vk::create_shader_pass_layout_impl(pass* pass)
{
    dt::vector<VkDescriptorSetLayout> layouts(DESCRIPTOR_TABLE_COUNT);
    for( u64 i = 0; i < DESCRIPTOR_TABLE_COUNT; i++ )
    {
        layouts.push_back(pass->get_descriptor_table((descriptor_table_type)i).get_impl<VkDescriptorSetLayout>());
    }

    VkPipelineLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    createInfo.setLayoutCount = u32_cast(DESCRIPTOR_TABLE_COUNT);
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
        binding.binding = curIndex++;
        binding.stageFlags = converters::get_shader_stage_flags_vk(slot.get_visibility());
        binding.descriptorCount = slot.get_array_size();
        binding.descriptorType = converters::get_descriptor_type_vk(slot.get_resource_type());

        bindings.push_back(binding);
    }

    for( const auto& slot : imageSlots )
    {
        VkDescriptorSetLayoutBinding binding{ };
        binding.binding = curIndex++;
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

void device_vk::destroy_shader_program(program* program)
{
    for( u64 passIdx = 0; passIdx < program->get_pass_count(); passIdx++ )
    {
        const pass& pass = program->get_pass(passIdx);

        // Destroy the tables first.
        for( u64 tableIdx = 0; tableIdx < DESCRIPTOR_TABLE_COUNT; tableIdx++ )
        {
            const descriptor_table_desc& table = pass.get_descriptor_table((descriptor_table_type)tableIdx);
            vkDestroyDescriptorSetLayout(m_device, table.get_impl<VkDescriptorSetLayout>(), nullptr);
        }

        // These two doesn't really matter what order
        vkDestroyPipelineLayout(m_device, pass.get_layout_impl<VkPipelineLayout>(), nullptr);
        vkDestroyPipeline(m_device, pass.get_impl<VkPipeline>(), nullptr);
    }
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
    return { "VK_KHR_swapchain" };
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