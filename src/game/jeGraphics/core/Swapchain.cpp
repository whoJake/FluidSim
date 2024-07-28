#include "Swapchain.h"
#include "Device.h"

namespace vk
{

Swapchain::Swapchain(Device& device,
                     VkSurfaceKHR                             surface,
                     const std::vector<VkPresentModeKHR>&     presentModePriority,
                     const std::vector<VkSurfaceFormatKHR>&   surfaceFormatPriority,
                     const VkExtent2D&                        extent,
                     const uint32_t                           imageCount,
                     const VkSurfaceTransformFlagBitsKHR      transform,
                     const std::set<VkImageUsageFlagBits>&    imageUsageFlags) :
    Swapchain(*this, device, surface, presentModePriority, surfaceFormatPriority, extent, imageCount, transform, imageUsageFlags)
{ }

Swapchain::Swapchain(Swapchain& oldSwapchain,
                     Device& device,
                     VkSurfaceKHR                             surface,
                     const std::vector<VkPresentModeKHR>&     presentModePriority,
                     const std::vector<VkSurfaceFormatKHR>&   surfaceFormatPriority,
                     const VkExtent2D&                        extent,
                     const uint32_t                           imageCount,
                     const VkSurfaceTransformFlagBitsKHR      transform,
                     const std::set<VkImageUsageFlagBits>&    imageUsageFlags) :
    Resource(VK_NULL_HANDLE, device),
    m_surface(surface),
    m_formatPriorityList(surfaceFormatPriority),
    m_presentModePriorityList(presentModePriority),
    m_imageUsageFlagBits(imageUsageFlags)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities{ };
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_gpu().get_handle(), surface, &surfaceCapabilities);

    uint32_t formatCount{ 0 };
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_gpu().get_handle(), surface, &formatCount, nullptr),
        "Failed to find supported surface formats for initialized device.");

    m_supportedFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_gpu().get_handle(), surface, &formatCount, m_supportedFormats.data());

    if( oldSwapchain.get_handle() == VK_NULL_HANDLE )
    {
        VULKAN_INFO("Found {} supported surface formats.", formatCount);
    }

    uint32_t presentModeCount{ 0 };
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_gpu().get_handle(), surface, &presentModeCount, nullptr),
        "Failed to find supported present modes for initialized device.");

    m_supportedPresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_gpu().get_handle(), surface, &presentModeCount, m_supportedPresentModes.data());

    if( oldSwapchain.get_handle() == VK_NULL_HANDLE )
    {
        VULKAN_INFO("Found {} supported present modes.", presentModeCount);
    }

    m_properties.oldSwapchain = oldSwapchain.get_handle();
    m_properties.imageCount = choose_image_count(imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
    m_properties.extent = choose_extent(extent, surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent, surfaceCapabilities.currentExtent);
    m_properties.surfaceFormat = choose_surface_format(surfaceFormatPriority, m_supportedFormats);
    m_properties.arrayLayers = choose_array_layer_count(1, surfaceCapabilities.maxImageArrayLayers);
    m_properties.imageUsage = choose_image_usage_flags(imageUsageFlags, surfaceCapabilities.supportedUsageFlags);
    // m_properties.transform = choose_transform(transform, surfaceCapabilities.supportedTransforms, surfaceCapabilities.currentTransform);
    m_properties.transform = surfaceCapabilities.currentTransform;
    m_properties.compositeAlpha = choose_composite_alpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, surfaceCapabilities.supportedCompositeAlpha);
    m_properties.presentMode = choose_present_mode(presentModePriority, m_supportedPresentModes);

    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.oldSwapchain = m_properties.oldSwapchain;    
    createInfo.minImageCount = m_properties.imageCount;
    createInfo.imageExtent = m_properties.extent;
    createInfo.imageColorSpace = m_properties.surfaceFormat.colorSpace;
    createInfo.imageFormat = m_properties.surfaceFormat.format;
    createInfo.imageArrayLayers = m_properties.arrayLayers;
    createInfo.imageUsage = m_properties.imageUsage;
    createInfo.preTransform = m_properties.transform;
    createInfo.compositeAlpha = m_properties.compositeAlpha;
    createInfo.presentMode = m_properties.presentMode;
    createInfo.surface = m_surface;

    VkResult result = vkCreateSwapchainKHR(m_device.get_handle(), &createInfo, nullptr, &m_handle);
    VK_CHECK(result, "Failed to create Swapchain.");

    uint32_t swapchainImageCount{ 0 };
    vkGetSwapchainImagesKHR(m_device.get_handle(), m_handle, &swapchainImageCount, nullptr);

    m_images.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(m_device.get_handle(), m_handle, &swapchainImageCount, m_images.data());
}

Swapchain::Swapchain(Swapchain&        oldSwapchain,
                     const VkExtent2D& extent) :
    Swapchain(oldSwapchain,
        oldSwapchain.m_device,
        oldSwapchain.m_surface,
        oldSwapchain.m_presentModePriorityList,
        oldSwapchain.m_formatPriorityList,
        extent,
        oldSwapchain.m_properties.imageCount,
        oldSwapchain.m_properties.transform,
        oldSwapchain.m_imageUsageFlagBits)
{ }

Swapchain::~Swapchain()
{
    vkDestroySwapchainKHR(m_device.get_handle(), m_handle, nullptr);
}

bool Swapchain::is_valid() const
{
    return m_handle != VK_NULL_HANDLE;
}

VkSurfaceKHR Swapchain::get_surface() const
{
    return m_surface;
}

VkResult Swapchain::acquire_next_image(uint32_t* imageIndex, VkSemaphore imageAcquiredSempahore, VkFence fence) const
{
    return vkAcquireNextImageKHR(m_device.get_handle(), m_handle, std::numeric_limits<uint64_t>::max(), imageAcquiredSempahore, fence, imageIndex);
}

const VkExtent2D& Swapchain::get_extent() const
{
    return m_properties.extent;
}

VkFormat Swapchain::get_format() const
{
    return m_properties.surfaceFormat.format;
}

const std::vector<VkImage>& Swapchain::get_images() const
{
    return m_images;
}

VkSurfaceTransformFlagBitsKHR Swapchain::get_transform() const
{
    return m_properties.transform;
}

VkImageUsageFlags Swapchain::get_usage() const
{
    return m_properties.imageUsage;
}

VkPresentModeKHR Swapchain::get_present_mode() const
{
    return m_properties.presentMode;
}

uint32_t Swapchain::choose_image_count(uint32_t requestedImageCount, uint32_t minSupportedImageCount, uint32_t maxSupportedImageCount) const
{
    uint32_t a = std::min(requestedImageCount, maxSupportedImageCount);
    return std::max(a, minSupportedImageCount);
}

VkExtent2D Swapchain::choose_extent(VkExtent2D requestedExtent, VkExtent2D minSupportedExtent, VkExtent2D maxSupportedExtent, VkExtent2D currentExtent) const
{
    // Way of implying that any extent is supported
    if( currentExtent.width == 0xFFFFFFFF )
        return requestedExtent;

    if( requestedExtent.width < 1 || requestedExtent.height < 1 )
    {
        // VULKAN_WARN("Swapchain image extent ({}, {}) is unsupported. Selecting ({}, {}).", requestedExtent.width, requestedExtent.height, currentExtent.width, currentExtent.height);
        return currentExtent;
    }

    VkExtent2D clampedExtent = requestedExtent;
    clampedExtent.width = std::min(clampedExtent.width, maxSupportedExtent.width);
    clampedExtent.width = std::max(clampedExtent.width, minSupportedExtent.width);

    clampedExtent.height = std::min(clampedExtent.height, maxSupportedExtent.height);
    clampedExtent.height = std::max(clampedExtent.height, minSupportedExtent.height);

    return clampedExtent;
}

VkSurfaceFormatKHR Swapchain::choose_surface_format(const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriority, const std::vector<VkSurfaceFormatKHR>& supportedSurfaceFormats) const
{
    for( size_t i = 0; i < surfaceFormatPriority.size(); i++ )
    {
        for( size_t j = 0; j < supportedSurfaceFormats.size(); j++ )
        {
            if( supportedSurfaceFormats[j].colorSpace == surfaceFormatPriority[i].colorSpace
                && supportedSurfaceFormats[j].format == surfaceFormatPriority[i].format )
            {
                return surfaceFormatPriority[i];
            }
        }
    }

    VULKAN_WARN("Couldn't find a supported surface format in priority list provided, defaulting to first supported format.");
    return supportedSurfaceFormats[0];
}

uint32_t Swapchain::choose_array_layer_count(uint32_t requestedArrayCount, uint32_t maxSupportedArrayLayers) const
{
    return std::min(requestedArrayCount, maxSupportedArrayLayers);
}

VkImageUsageFlags Swapchain::choose_image_usage_flags(const std::set<VkImageUsageFlagBits>& requestedUsageFlags, VkImageUsageFlags supportedUsageFlags) const
{
    VkImageUsageFlags result{ };

    for( VkImageUsageFlagBits flag : requestedUsageFlags)
    {
        if( flag & supportedUsageFlags )
        {
            result |= flag;
        }
    }

    if( !result )
        QUITFMT("None of the requested swapchain image usage flags are supported.");

    return result;
}

VkSurfaceTransformFlagsKHR Swapchain::choose_transform(VkSurfaceTransformFlagBitsKHR requestedTransform, VkSurfaceTransformFlagsKHR supportedTransforms, VkSurfaceTransformFlagsKHR currentTransform) const
{
    if( requestedTransform & supportedTransforms )
        return requestedTransform;

    VULKAN_WARN("Requested surface transform is not supported. Choosing default transform instead.");
    return currentTransform;
}

VkCompositeAlphaFlagBitsKHR Swapchain::choose_composite_alpha(VkCompositeAlphaFlagBitsKHR requestedCompositeAlpha, VkCompositeAlphaFlagsKHR supportedCompositeAlpha) const
{
    if( requestedCompositeAlpha & supportedCompositeAlpha )
        return requestedCompositeAlpha;

    VULKAN_WARN("Requested composite alpha is not supported. Defaulting to VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR");
    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

VkPresentModeKHR Swapchain::choose_present_mode(const std::vector<VkPresentModeKHR>& presentModePriority, const std::vector<VkPresentModeKHR>& supportedPresentModes) const
{
    for( size_t i = 0; i < presentModePriority.size(); i++ )
    {
        auto itFind = std::find(supportedPresentModes.begin(), supportedPresentModes.end(), presentModePriority[i]);
        if( itFind != supportedPresentModes.end() )
            return presentModePriority[i];
    }

    VULKAN_WARN("Couldn't find a supported present mode in priority list provided, defaulting to first supported present mode.");
    return supportedPresentModes[0];
}

} // vk