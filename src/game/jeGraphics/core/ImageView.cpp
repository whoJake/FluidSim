#include "ImageView.h"
#include "Device.h"
#include "Image.h"

namespace vk
{

ImageView::ImageView(Image&          image,
                     VkImageViewType viewType,
                     VkFormat        format,
                     uint32_t        baseMipLevel,
                     uint32_t        baseArrayLevel,
                     uint32_t        mipCount,
                     uint32_t        arrayCount) :
    Resource(VK_NULL_HANDLE, image.get_device()),
    m_image(&image),
    m_format(format),
    m_subresourceRange()
{
    if( format == VK_FORMAT_UNDEFINED )
        m_format = m_image->get_format();

    m_subresourceRange.baseMipLevel = baseMipLevel;
    m_subresourceRange.baseArrayLayer = baseArrayLevel;
    m_subresourceRange.levelCount = mipCount == 0 ? m_image->get_subresource().mipLevel : mipCount;
    m_subresourceRange.layerCount = arrayCount == 0 ? m_image->get_subresource().arrayLayer : arrayCount;
    if( is_depth_format(m_format) )
        m_subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else
        m_subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    createInfo.image = m_image->get_handle();
    createInfo.viewType = viewType;
    createInfo.format = m_format;
    createInfo.subresourceRange = m_subresourceRange;

    VkResult result = vkCreateImageView(m_device.get_handle(), &createInfo, nullptr, &m_handle);
    VK_CHECK(result, "Failed to create Image View.");

    m_image->get_views().emplace(this);
}

ImageView::~ImageView()
{
    vkDestroyImageView(m_device.get_handle(), m_handle, nullptr);
}

ImageView::ImageView(ImageView&& other) :
    Resource(std::move(other)),
    m_image(other.m_image),
    m_format(other.m_format),
    m_subresourceRange(other.m_subresourceRange)
{
    auto& views = m_image->get_views();
    views.erase(&other);
    views.emplace(this);

    other.m_handle = VK_NULL_HANDLE;
}

const Image& ImageView::get_image() const
{
    return *m_image;
}

void ImageView::set_image(Image& image)
{
    m_image = &image;
}

VkFormat ImageView::get_format() const
{
    return m_format;
}

VkImageSubresourceRange ImageView::get_subresource_range() const
{
    return m_subresourceRange;
}

VkImageSubresourceLayers ImageView::get_subresource_layers() const
{
    VkImageSubresourceLayers result{ };
    result.aspectMask = m_subresourceRange.aspectMask;
    result.baseArrayLayer = m_subresourceRange.baseArrayLayer;
    result.layerCount = m_subresourceRange.layerCount;
    result.mipLevel = m_subresourceRange.baseMipLevel;
    return result;
}

} // vk