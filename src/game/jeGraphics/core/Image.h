#pragma once

#include "vkcommon.h"
#include "Resource.h"

namespace vk
{

class ImageView;

/**
* @brief Wrapper class for VkImage
*/
class Image : public Resource<VkImage>
{
public:
    /**
    * @brief Create and allocate VkImage with given properties
    */
    Image(Device&               device,
          const VkExtent3D&     extent,
          VkFormat              format,
          VkImageUsageFlags     usage,
          VmaMemoryUsage        memoryUsage,
          VkSampleCountFlagBits sampleCount      = VK_SAMPLE_COUNT_1_BIT,
          uint32_t              mipLevels        = 1,
          uint32_t              arrayLayers      = 1,
          VkImageTiling         tiling           = VK_IMAGE_TILING_OPTIMAL,
          VkImageCreateFlags    flags            = 0,
          uint32_t              numQueueFamilies = 0,
          const uint32_t*       pQueueFamilies   = nullptr);

    /**
    * @brief Create Image class using an existing VkImage ( for swapchains )
    */
    Image(Device&               device,
          VkImage               handle,
          const VkExtent3D&     extent,
          VkFormat              format,
          VkImageUsageFlags     usage,
          VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    ~Image() override;

    Image(const Image& other) = delete;
    Image(Image&& other);
    Image& operator=(const Image& other) = delete;
    Image& operator=(Image&& other) = delete;

    const VmaAllocation& get_allocation() const;

    uint8_t* map();

    void unmap();

    VkImageType get_type() const;

    const VkExtent3D& get_extent() const;

    VkFormat get_format() const;

    VkSampleCountFlagBits get_sample_count() const;

    VkImageUsageFlags get_usage() const;

    VkImageTiling get_tiling() const;

    VkImageSubresource get_subresource() const;

    uint32_t get_array_layer_count() const;

    std::unordered_set<ImageView*>& get_views();
private:
    VmaAllocation m_allocation{ };

    VkImageType m_type{ };
    VkExtent3D m_extent{ };
    VkFormat m_format{ };
    VkImageUsageFlags m_usage{ };

    VkSampleCountFlagBits m_sampleCount{ };
    VkImageTiling m_tiling{ };

    VkImageSubresource m_subresource{ };
    uint32_t m_arrayLayerCount{ };

    std::unordered_set<ImageView*> m_views;

    uint8_t* m_mappedData = nullptr;
    bool m_mapped = false;
};

} // vk