#pragma once

#include "application/WindowedApplication.h"
#include "platform/events/Event.h"

#include "render/ImageRenderer.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderTarget.h"

#include "image/Image.h"
#include "render/Camera.h"
#include "platform/events/WindowEvent.h"
#include "render/Blas.h"

class SandboxApp : public WindowedApplication
{
public:
    void on_app_startup() override;
    void on_app_shutdown() override;

    void update() override;

    void on_event(Event& e) override;
private:
    bool on_window_resize(WindowResizeEvent& e);
    void calculate_delta_time();

    void parse_inputs();
private:
    std::unique_ptr<vk::RenderContext> m_context;
    std::unique_ptr<ImageRenderer> m_renderer;

    std::unique_ptr<vk::Image> m_image;
    std::unique_ptr<Image> m_cpuImage;

    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Blas> m_blas;

    std::chrono::steady_clock::time_point m_lastFrameBeginTime{ std::chrono::high_resolution_clock::now() };
    double m_deltaTime{ 0.f };
};