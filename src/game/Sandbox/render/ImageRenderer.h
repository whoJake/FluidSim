#pragma once

#include "rendering/RenderContext.h"
#include "rendering/RenderFrame.h"
#include "rendering/RenderTarget.h"
#include "implementations/ImGuiContext.h"
#include "platform/Window.h"

class ImageRenderer
{
public:
    ImageRenderer(vk::RenderContext& context, Window* window, mygui::Context** imguiContext);
    ~ImageRenderer();

    void render_image(vk::Image* image);
private:
    void initialize(Window* window, mygui::Context** imguiContext);
private:
    vk::RenderContext& m_context;
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::unique_ptr<vk::Image> m_blitTarget;
    mygui::Context* m_imguiContext;
};