#pragma once

#include "platform/Window.h"
#include "rendering/RenderContext.h"
#include "system/timer.h"

namespace mygui
{

class Context
{
public:
    Context(Window* glfwWindow, vk::RenderContext* renderContext, vk::RenderPass* renderPass);
    ~Context();

    void begin_frame();

    void end_frame();

    void render(vk::CommandBuffer* commandBuffer);
private:
    Window* m_window;
    vk::RenderContext* m_renderContext;

    VkDescriptorPool m_pool;
    sys::moment m_frameStart;
};

} // mygui