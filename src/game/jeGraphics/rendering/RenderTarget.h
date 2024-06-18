#pragma once

#include "vkcommon.h"
#include "core/Image.h"
#include "core/ImageView.h"
#include <functional>

namespace vk
{

class Device;

struct Attachment
{
    VkFormat format{ VK_FORMAT_UNDEFINED };
    VkSampleCountFlagBits sampleCount{ VK_SAMPLE_COUNT_1_BIT };
    VkImageUsageFlags usage{ VK_IMAGE_USAGE_SAMPLED_BIT };
    VkImageLayout initialLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

    Attachment() = default;
    Attachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage);
};

class RenderTarget
{
public:
    using CreateFunc = std::function<std::unique_ptr<RenderTarget>(Image&&)>;

    RenderTarget(std::vector<Image>&& images);
    RenderTarget(std::vector<ImageView>&& imageViews);

    RenderTarget(const RenderTarget& other) = delete;
    RenderTarget(RenderTarget&& other) = delete;
    RenderTarget& operator=(const RenderTarget& other) = delete;
    RenderTarget& operator=(RenderTarget&& other) = delete;

    const VkExtent2D& get_extent() const;

    const std::vector<ImageView>& get_image_views() const;

    const std::vector<Attachment>& get_attachments() const;

    const std::vector<uint32_t>& get_input_attachments() const;

    void set_input_attachments(std::vector<uint32_t>& attachments);

    const std::vector<uint32_t>& get_output_attachments() const;

    void set_output_attachments(std::vector<uint32_t>& attachments);

    void set_layout(uint32_t attachment, VkImageLayout layout);

    VkImageLayout get_layout(uint32_t attachment) const;

    static std::unique_ptr<RenderTarget> default_create_function(Image&& image);

    static std::unique_ptr<RenderTarget> no_depth_create_function(Image&& image);
private:
    Device& m_device;

    VkExtent2D m_extent;

    std::vector<Image> m_images;

    std::vector<ImageView> m_imageViews;

    std::vector<Attachment> m_attachments;

    std::vector<uint32_t> m_inputAttachments{ };

    std::vector<uint32_t> m_outputAttachments{ 0 };
};

} // vk