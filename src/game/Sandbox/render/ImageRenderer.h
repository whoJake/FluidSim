#pragma once

#include "rendering/RenderContext.h"
#include "rendering/RenderFrame.h"
#include "rendering/RenderTarget.h"

class ImageRenderer
{
public:
    ImageRenderer(vk::RenderContext& context);
    ~ImageRenderer();

    void render_image(vk::Image* image);
private:
    void initialize();
private:
    vk::RenderContext& m_context;
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::unique_ptr<vk::Image> m_blitTarget;
};