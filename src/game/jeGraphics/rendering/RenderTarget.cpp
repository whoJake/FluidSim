#include "RenderTarget.h"

#include "core/Device.h"
#include <algorithm>
#include <functional>

namespace vk
{

Attachment::Attachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage) :
    format(format),
    sampleCount(sampleCount),
    usage(usage)
{ }

RenderTarget::RenderTarget(std::vector<Image>&& images) :
    m_device(images.back().get_device()),
    m_images(std::move(images))
{
    if( m_images.size() == 0 )
        QUITFMT("Render target requires at least one image.");

    std::function<VkExtent2D(const Image&)> image_to_extent_2D =
        [](const Image& image) -> VkExtent2D
        {
            return VkExtent2D{ image.get_extent().width, image.get_extent().height };
        };

    // Checks to see if there is only one unique extent size
    std::hash<VkExtent2D> hasher;
    m_extent = image_to_extent_2D(m_images[0]);
    size_t uniqueHash = hasher(m_extent);

    for( auto& image : m_images )
    {
        if( image.get_type() != VK_IMAGE_TYPE_2D )
            QUITFMT("Images of a RenderTarget must be of type 2D.");

        VkExtent2D extent = image_to_extent_2D(image);
        if( hasher(extent) != uniqueHash )
            QUITFMT("Cannot have more than one extent size in a Render Target.");

        m_imageViews.emplace_back(image, VK_IMAGE_VIEW_TYPE_2D);
        m_attachments.emplace_back(Attachment(image.get_format(), image.get_sample_count(), image.get_usage()));
    }
}

RenderTarget::RenderTarget(std::vector<ImageView>&& imageViews) :
    m_device(imageViews.back().get_device()),
    m_images(),
    m_imageViews(std::move(imageViews))
{
    if( m_imageViews.size() == 0 )
        QUITFMT("Render target requires at least one image view.");

    std::function<VkExtent2D(const ImageView&)> imageview_to_extent_2D =
        [](const ImageView& view) -> VkExtent2D
        {
            const VkExtent3D extent = view.get_image().get_extent();
            const uint32_t mip = view.get_subresource_range().baseMipLevel;
            return VkExtent2D{ extent.width >> mip, extent.height >> mip };
        };

    /// Checks to see if there is only one unique exxtent size
    std::hash<VkExtent2D> hasher;
    m_extent = imageview_to_extent_2D(m_imageViews[0]);
    size_t uniqueHash = hasher(m_extent);

    for( auto& imageview : m_imageViews )
    {
        VkExtent2D extent = imageview_to_extent_2D(imageview);

        if( hasher(extent) != uniqueHash )
            QUITFMT("Cannot have more than one extent size in a Render Target.");

        const Image& image = imageview.get_image();
        m_attachments.emplace_back(Attachment(image.get_format(), image.get_sample_count(), image.get_usage()));
    }
}

const VkExtent2D& RenderTarget::get_extent() const
{
    return m_extent;
}

const std::vector<ImageView>& RenderTarget::get_image_views() const
{
    return m_imageViews;
}

const std::vector<Attachment>& RenderTarget::get_attachments() const
{
    return m_attachments;
}

const std::vector<uint32_t>& RenderTarget::get_input_attachments() const
{
    return m_inputAttachments;
}

void RenderTarget::set_input_attachments(std::vector<uint32_t>& attachments)
{
    m_inputAttachments = attachments;
}

const std::vector<uint32_t>& RenderTarget::get_output_attachments() const
{
    return m_outputAttachments;
}

void RenderTarget::set_output_attachments(std::vector<uint32_t>& attachments)
{
    m_outputAttachments = attachments;
}

void RenderTarget::set_layout(uint32_t attachment, VkImageLayout layout)
{
    m_attachments[attachment].initialLayout = layout;
}

VkImageLayout RenderTarget::get_layout(uint32_t attachment) const
{
    return m_attachments[attachment].initialLayout;
}

std::unique_ptr<RenderTarget> RenderTarget::default_create_function(Image&& image)
{
    VkFormat depthFormat = get_suitable_depth_format(image.get_device().get_gpu().get_handle());
    Image depthImage(image.get_device(),
        image.get_extent(),
        depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    std::vector<Image> targetImages;
    targetImages.push_back(std::move(image));
    targetImages.push_back(std::move(depthImage));

    return std::make_unique<RenderTarget>(std::move(targetImages));
}

std::unique_ptr<RenderTarget> RenderTarget::no_depth_create_function(Image&& image)
{
    std::vector<Image> targetImages;
    targetImages.push_back(std::move(image));

    return std::make_unique<RenderTarget>(std::move(targetImages));
}

} // vk