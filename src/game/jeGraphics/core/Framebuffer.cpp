#include "Framebuffer.h"
#include "Device.h"

namespace vk
{

Framebuffer::Framebuffer(Device& device,
                         const RenderTarget& renderTarget,
                         const RenderPass& renderPass) :
    Resource(VK_NULL_HANDLE, device),
    m_extent(renderTarget.get_extent())
{
    std::vector<VkImageView> attachments;
    for( auto& view : renderTarget.get_image_views() )
    {
        attachments.emplace_back(view.get_handle());
    }

    VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    createInfo.renderPass = renderPass.get_handle();
    createInfo.attachmentCount = to_u32(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.width = m_extent.width;
    createInfo.height = m_extent.height;
    createInfo.layers = 1;

    VkResult createResult = vkCreateFramebuffer(m_device.get_handle(), &createInfo, nullptr, &m_handle);
    VK_CHECK(createResult, "Failed to create Framebuffer.");
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(m_device.get_handle(), m_handle, nullptr);
}

Framebuffer::Framebuffer(Framebuffer&& other) :
    Resource(std::move(other)),
    m_extent(other.m_extent)
{
    other.m_handle = VK_NULL_HANDLE;
}

const VkExtent2D Framebuffer::get_extent() const
{
    return m_extent;
}

} // vk