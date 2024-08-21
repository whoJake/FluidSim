#pragma once

#include "vkcommon.h"
#include "Resource.h"

namespace vk
{

class Device;
struct Attachment;

struct SubpassInfo
{
    std::vector<uint32_t> inputAttachments;
    std::vector<uint32_t> outputAttachments;
    std::vector<uint32_t> colorResolveAttachments;
    bool disableDepthStencilAttachment;
    uint32_t depthStencilResolveAttachment;
    VkResolveModeFlagBits depthStencilResolveMode;
    std::string debugName;
};

class RenderPass : public Resource<VkRenderPass>
{
public:
    RenderPass(Device&                           device,
               const std::vector<Attachment>&    attachments,
               const std::vector<LoadStoreInfo>& loadStoreInfos,
               const std::vector<SubpassInfo>&   subpasses);
    ~RenderPass();

    RenderPass(const RenderPass& other) = delete;
    RenderPass(RenderPass&& other);
    RenderPass& operator=(const RenderPass& other) = delete;
    RenderPass& operator=(RenderPass&& other) = delete;

    const uint32_t get_color_output_count(uint32_t subpassIndex) const;

    const VkExtent2D get_render_area_granularity() const;
private:
    void set_attachment_layouts(std::vector<VkSubpassDescription>& subpassDescriptions, std::vector<VkAttachmentDescription>& attachmentDescriptions);
private:
    size_t m_subpassCount;

    std::vector<uint32_t> m_colorOutputCount;
};

} // vk