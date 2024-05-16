#pragma once

#include "vkcommon.h"
#include "Resource.h"
#include "RenderPass.h"
#include "rendering/RenderTarget.h"

namespace vk
{

class Device;

class Framebuffer : public Resource<VkFramebuffer>
{
public:
    Framebuffer(Device&             device,
                const RenderTarget& renderTarget,
                const RenderPass&   renderPass);
    ~Framebuffer();

    Framebuffer(const Framebuffer& other) = delete;
    Framebuffer(Framebuffer&& other);
    Framebuffer& operator=(const Framebuffer& other) = delete;
    Framebuffer& operator=(Framebuffer&& other) = delete;

    const VkExtent2D get_extent() const;
private:
    VkExtent2D m_extent;
};

} // vk
