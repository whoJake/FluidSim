#include "RenderPass.h"
#include "Device.h"
#include "rendering/RenderTarget.h"

namespace vk
{

RenderPass::RenderPass(Device&                           device,
                       const std::vector<Attachment>&    attachments,
                       const std::vector<LoadStoreInfo>& loadStoreInfos,
                       const std::vector<SubpassInfo>&   subpasses) :
    Resource(VK_NULL_HANDLE, device),
    m_subpassCount(subpasses.size()),
    m_colorOutputCount()
{
    if( subpasses.size() == 0 )
        QUITFMT("A renderpass must be built with atleast one subpass.");

    /// Setup attachment descriptions
    std::vector<VkAttachmentDescription> attachmentDescriptions{ };
    for( size_t i = 0; i < attachments.size(); i++ )
    {
        VkAttachmentDescription desc{ };
        desc.format = attachments[i].format;
        desc.samples = attachments[i].sampleCount;
        desc.initialLayout = attachments[i].initialLayout;
        desc.finalLayout = is_depth_format(desc.format)
            ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        if( i < loadStoreInfos.size() )
        {
            desc.loadOp = loadStoreInfos[i].loadOp;
            desc.storeOp = loadStoreInfos[i].storeOp;
            desc.stencilLoadOp = loadStoreInfos[i].loadOp;
            desc.stencilStoreOp = loadStoreInfos[i].storeOp;
        }

        attachmentDescriptions.push_back(desc);
    }

    /// Seperate and store all attachments
    std::vector<std::vector<VkAttachmentReference>> inputAttachments(m_subpassCount);
    std::vector<std::vector<VkAttachmentReference>> colorAttachments(m_subpassCount);
    std::vector<std::vector<VkAttachmentReference>> colorResolveAttachments(m_subpassCount);
    std::vector<std::vector<VkAttachmentReference>> depthStencilAttachments(m_subpassCount);
    std::vector<std::vector<VkAttachmentReference>> depthResolveAttachments(m_subpassCount);

    for( size_t i = 0; i < subpasses.size(); i++ )
    {
        const SubpassInfo& subpass = subpasses[i];

        /// Input attachments
        for( uint32_t attachment : subpass.inputAttachments )
        {
            VkImageLayout defaultLayout = is_depth_format(attachmentDescriptions[attachment].format)
                ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkImageLayout initialLayout = attachments[attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED
                ? defaultLayout
                : attachments[attachment].initialLayout;

            inputAttachments[i].push_back(VkAttachmentReference{ attachment, initialLayout });
        }

        /// Color attachments
        for( uint32_t attachment : subpass.outputAttachments )
        {
            VkImageLayout initialLayout = attachments[attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED
                ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
                attachments[attachment].initialLayout;
            VkAttachmentDescription& description = attachmentDescriptions[attachment];
            if( !is_depth_format(description.format) )
                colorAttachments[i].push_back(VkAttachmentReference{ attachment, initialLayout });
        }

        /// Color resolve Attachments
        for( uint32_t attachment : subpass.colorResolveAttachments )
        {
            VkImageLayout initialLayout = attachments[attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED
                ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                : attachments[attachment].initialLayout;

            colorResolveAttachments[i].push_back(VkAttachmentReference{ attachment, initialLayout });
        }

        /// Depth Stencil / Depth Resolve Attachments
        if( !subpass.disableDepthStencilAttachment )
        {
            // Makes assumption that depth stencil attachment comes before depth resolve
            auto it = std::find_if(attachments.begin(), attachments.end(), [](const Attachment& attachment) { return is_depth_format(attachment.format); });
            if( it != attachments.end() )
            {
                uint32_t depthStencilIndex = to_u32(std::distance(attachments.begin(), it));
                VkImageLayout initialLayout = it->initialLayout == VK_IMAGE_LAYOUT_UNDEFINED
                    ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                    : it->initialLayout;

                depthStencilAttachments[i].push_back(VkAttachmentReference{ depthStencilIndex, initialLayout });
                if( subpass.depthStencilResolveMode != VK_RESOLVE_MODE_NONE )
                {
                    uint32_t depthResolveIndex = subpass.depthStencilResolveAttachment;
                    initialLayout = attachments[depthResolveIndex].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED
                        ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                        : attachments[depthResolveIndex].initialLayout;

                    depthResolveAttachments[i].push_back(VkAttachmentReference{ depthResolveIndex, initialLayout });
                }
            }
        }
    }

    /// Build subpass descriptions
    std::vector<VkSubpassDescription> subpassDescriptions{ };
    subpassDescriptions.reserve(m_subpassCount);

    for( size_t i = 0; i < subpasses.size(); i++ )
    {
        const SubpassInfo& subpass = subpasses[i];

        VkSubpassDescription desc{ };
        desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        desc.inputAttachmentCount = to_u32(inputAttachments[i].size());
        desc.pInputAttachments = (!inputAttachments[i].empty()) ? inputAttachments[i].data() : nullptr;

        desc.colorAttachmentCount = to_u32(colorAttachments[i].size());
        desc.pColorAttachments = (!colorAttachments[i].empty()) ? colorAttachments[i].data() : nullptr;

        desc.pResolveAttachments = (!colorResolveAttachments[i].empty()) ? colorResolveAttachments[i].data() : nullptr;

        desc.pDepthStencilAttachment = nullptr;
        if( !depthStencilAttachments[i].empty() )
        {
            desc.pDepthStencilAttachment = depthStencilAttachments[i].data();

            // idk lol
            if( !depthResolveAttachments[i].empty() )
            {
                auto& ref = depthResolveAttachments[i][0];
                if( attachmentDescriptions[ref.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED ) {
                    attachmentDescriptions[ref.attachment].initialLayout = ref.layout;
                }
            }
        }

        subpassDescriptions.push_back(desc);
    }

    set_attachment_layouts(subpassDescriptions, attachmentDescriptions);

    m_colorOutputCount.reserve(m_subpassCount);
    for( size_t i = 0; i < m_subpassCount; i++ )
    {
        m_colorOutputCount.push_back(to_u32(colorAttachments[i].size()));
    }

    std::vector<VkSubpassDependency> subpassDependencies(m_subpassCount - 1);
    for( size_t i = 0; i < subpassDependencies.size(); i++ )
    {
        subpassDependencies[i].srcSubpass = to_u32(i);
        subpassDependencies[i].dstSubpass = to_u32(i + 1);
        subpassDependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        subpassDependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[i].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        subpassDependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    createInfo.attachmentCount = to_u32(attachmentDescriptions.size());
    createInfo.pAttachments = attachmentDescriptions.data();
    createInfo.subpassCount = to_u32(subpassDescriptions.size());
    createInfo.pSubpasses = subpassDescriptions.data();
    createInfo.dependencyCount = to_u32(subpassDependencies.size());
    createInfo.pDependencies = subpassDependencies.data();

    VkResult createResult = vkCreateRenderPass(m_device.get_handle(), &createInfo, nullptr, &m_handle);
    VK_CHECK(createResult, "Failed to create Render Pass.");
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(m_device.get_handle(), m_handle, nullptr);
}

RenderPass::RenderPass(RenderPass&& other) :
    Resource(std::move(other)),
    m_subpassCount(other.m_subpassCount),
    m_colorOutputCount(other.m_colorOutputCount)
{
    other.m_handle = VK_NULL_HANDLE;
}

const uint32_t RenderPass::get_color_output_count(uint32_t subpassIndex) const
{
    return m_colorOutputCount[subpassIndex];
}

const VkExtent2D RenderPass::get_render_area_granularity() const {
    VkExtent2D result{ };
    vkGetRenderAreaGranularity(m_device.get_handle(), m_handle, &result);
    return result;
}

void RenderPass::set_attachment_layouts(std::vector<VkSubpassDescription>& subpassDescriptions, std::vector<VkAttachmentDescription>& attachmentDescriptions)
{
    return;

    /// Initial layout becomes layout of first subpass that that attachment
    for( auto& subpass : subpassDescriptions )
    {
        for( size_t i = 0; i < subpass.colorAttachmentCount; i++ )
        {
            auto& ref = subpass.pColorAttachments[i];
            if( attachmentDescriptions[ref.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED )
                attachmentDescriptions[ref.attachment].initialLayout = ref.layout;
        }

        for( size_t i = 0; i < subpass.inputAttachmentCount; i++ )
        {
            auto& ref = subpass.pInputAttachments[i];
            if( attachmentDescriptions[ref.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED )
                attachmentDescriptions[ref.attachment].initialLayout = ref.layout;
        }

        if( subpass.pDepthStencilAttachment )
        {
            auto& ref = *subpass.pDepthStencilAttachment;
            if( attachmentDescriptions[ref.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED )
                attachmentDescriptions[ref.attachment].initialLayout = ref.layout;
        }

        if( subpass.pResolveAttachments )
        {
            for( size_t i = 0; i < subpass.colorAttachmentCount; i++ )
            {
                auto& ref = subpass.pResolveAttachments[i];
                if( attachmentDescriptions[ref.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED )
                    attachmentDescriptions[ref.attachment].initialLayout = ref.layout;
            }
        }
    }


    /// Final layout becomes layout of last subpass
    {
        auto& subpass = subpassDescriptions.back();

        for( size_t i = 0; i < subpass.colorAttachmentCount; i++ )
        {
            auto& ref = subpass.pColorAttachments[i];
            attachmentDescriptions[ref.attachment].finalLayout = ref.layout;
        }

        for( size_t i = 0; i < subpass.inputAttachmentCount; i++ )
        {
            auto& ref = subpass.pInputAttachments[i];
            attachmentDescriptions[ref.attachment].finalLayout = ref.layout;

            // Input attachment has built in depth format
            if( is_depth_format(attachmentDescriptions[ref.attachment].format) )
                subpass.pDepthStencilAttachment = nullptr;
        }

        if( subpass.pDepthStencilAttachment )
        {
            auto& ref = *subpass.pDepthStencilAttachment;
            attachmentDescriptions[ref.attachment].finalLayout = ref.layout;
        }

        if( subpass.pResolveAttachments )
        {
            for( size_t i = 0; i < subpass.colorAttachmentCount; i++ )
            {
                auto& ref = subpass.pResolveAttachments[i];
                attachmentDescriptions[ref.attachment].finalLayout = ref.layout;
            }
        }
    }

}

} // vk