#pragma once

#include "vkcommon.h"
#include "Resource.h"
#include <set>

namespace vk
{

class Device;

struct SwapchainProperties
{
    VkSwapchainKHR                 oldSwapchain;
    uint32_t                       imageCount{ 2 };
    VkExtent2D                     extent{ };
    VkSurfaceFormatKHR             surfaceFormat{ };
    uint32_t                       arrayLayers;
    VkImageUsageFlags              imageUsage;
    VkSurfaceTransformFlagBitsKHR  transform;
    VkCompositeAlphaFlagBitsKHR    compositeAlpha;
    VkPresentModeKHR               presentMode;
};

class Swapchain : public Resource<VkSwapchainKHR>
{
public:
    Swapchain(Swapchain&                               oldSwapchain,
              Device&                                  device,
              VkSurfaceKHR                             surface,
              const std::vector<VkPresentModeKHR>&     presentModePriority   = { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR },
              const std::vector<VkSurfaceFormatKHR>&   surfaceFormatPriority = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} },
              const VkExtent2D&                        extent                = { },
              const uint32_t                           imageCount            = { 2 },
              const VkSurfaceTransformFlagBitsKHR      transform             = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
              const std::set<VkImageUsageFlagBits>&    imageUsageFlags       = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT });

    Swapchain(Device&                                  device,
              VkSurfaceKHR                             surface,
              const std::vector<VkPresentModeKHR>&     presentModePriority   = { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR },
              const std::vector<VkSurfaceFormatKHR>&   surfaceFormatPriority = { {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} },
              const VkExtent2D&                        extent                = { },
              const uint32_t                           imageCount            = { 2 },
              const VkSurfaceTransformFlagBitsKHR      transform             = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
              const std::set<VkImageUsageFlagBits>&    imageUsageFlags       = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT });

    Swapchain(Swapchain&                               oldSwapchain,
              const VkExtent2D&                        extent);

    ~Swapchain();

    bool is_valid() const;

    VkSurfaceKHR get_surface() const;

    VkResult acquire_next_image(uint32_t* imageIndex, VkSemaphore imageAcquiredSemaphore, VkFence fence = VK_NULL_HANDLE) const;

    const VkExtent2D& get_extent() const;

    VkFormat get_format() const;

    const std::vector<VkImage>& get_images() const;

    VkSurfaceTransformFlagBitsKHR get_transform() const;

    VkImageUsageFlags get_usage() const;

    VkPresentModeKHR get_present_mode() const;
private:
    VkSurfaceKHR m_surface;

    std::vector<VkImage> m_images;

    SwapchainProperties m_properties;

    std::vector<VkSurfaceFormatKHR> m_supportedFormats;

    std::vector<VkPresentModeKHR> m_supportedPresentModes;

    std::vector<VkSurfaceFormatKHR> m_formatPriorityList;

    std::vector<VkPresentModeKHR> m_presentModePriorityList;

    std::set<VkImageUsageFlagBits> m_imageUsageFlagBits;
private:
    uint32_t choose_image_count(uint32_t requestedImageCount, uint32_t minSupportedImageCount, uint32_t maxSupportedImageCount) const;

    VkExtent2D choose_extent(VkExtent2D requestedExtent, VkExtent2D minSupportedExtent, VkExtent2D maxSupportedExtent, VkExtent2D currentExtent) const;

    VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriority, const std::vector<VkSurfaceFormatKHR>& supportedSurfaceFormats) const;

    uint32_t choose_array_layer_count(uint32_t requestedArrayCount, uint32_t maxSupportedArrayLayers) const;

    VkImageUsageFlags choose_image_usage_flags(const std::set<VkImageUsageFlagBits>& requestedUsageFlags, VkImageUsageFlags supportedUsageFlags) const;

    VkSurfaceTransformFlagsKHR choose_transform(VkSurfaceTransformFlagBitsKHR requestedTransform, VkSurfaceTransformFlagsKHR supportedTransforms, VkSurfaceTransformFlagsKHR currentTransform) const;

    VkCompositeAlphaFlagBitsKHR choose_composite_alpha(VkCompositeAlphaFlagBitsKHR requestedCompositeAlpha, VkCompositeAlphaFlagsKHR supportedCompositeAlpha) const;

    VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& presentModePriority, const std::vector<VkPresentModeKHR>& supportedPresentModes) const;
};

} // vk