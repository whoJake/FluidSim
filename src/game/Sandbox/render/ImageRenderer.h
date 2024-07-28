#pragma once

#include "rendering/RenderContext.h"
#include "rendering/RenderFrame.h"
#include "rendering/RenderTarget.h"
#include "implementations/ImGuiContext.h"
#include "platform/Window.h"
#include "graphics/Material.h"

extern glm::vec2 g_offset;
extern glm::vec2 g_scale;
extern glm::vec3 g_color;

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

    std::unique_ptr<graphics::Shader> m_shader;
    std::unique_ptr<graphics::Material> m_material;
};