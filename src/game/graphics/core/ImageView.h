#pragma once

#include "vkcommon.h"
#include "Resource.h"

namespace vk
{

class Image;

class ImageView : public Resource<VkImageView>
{
public:
    ImageView(Image&          image,
              VkImageViewType viewType,
              VkFormat        format         = VK_FORMAT_UNDEFINED,
              uint32_t        baseMipLevel   = 0,
              uint32_t        baseArrayLevel = 0,
              uint32_t        mipCount       = 0,
              uint32_t        arrayCount     = 0);

    ~ImageView() override;

    ImageView(const ImageView& other) = delete;
    ImageView(ImageView&& other);
    ImageView& operator=(const ImageView& other) = delete;
    ImageView& operator=(ImageView&& other) = delete;

    const Image& get_image() const;

    /**
    * Updates the image that this ImageView looks into
    */
    void set_image(Image& image);

    VkFormat get_format() const;

    VkImageSubresourceRange get_subresource_range() const;

    VkImageSubresourceLayers get_subresource_layers() const;
private:
    Image* m_image;
    VkFormat m_format;
    VkImageSubresourceRange m_subresourceRange;
};

} // vk