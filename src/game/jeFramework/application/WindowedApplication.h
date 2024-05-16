#pragma once

#include "ApplicationBase.h"
#include "platform/events/Event.h"
#include "rendering/RenderContext.h"
#include "core/Pipeline.h"

class WindowedApplication: public ApplicationBase
{
public:
    WindowedApplication();
    WindowedApplication(const Window::Properties& properties);
    ~WindowedApplication();

    ExitFlags app_main() final;

    /// <summary>
    /// Ran after initialization of the app has occurred, but before the first call to update().
    /// </summary>
    virtual void on_app_startup() { };

    virtual void update(double deltaTime) { };

    virtual void on_event(Event& e) { };

    vk::RenderContext& get_render_context();

    Window& get_window();

    inline double get_delta_time() const { return m_deltaTime; }
protected:
    virtual std::vector<const char*> request_instance_extensions() const;

    virtual std::vector<const char*> request_validation_layers() const;

    virtual std::vector<const char*> request_device_extensions() const;

    virtual std::vector<VkPresentModeKHR> request_swapchain_present_mode() const;

    virtual std::vector<VkSurfaceFormatKHR> request_swapchain_format() const;

    virtual VkPhysicalDeviceFeatures request_physical_device_feature_set() const;
private:
    bool create_window();
    bool create_window(Window::Properties& properties);

    bool create_render_context();

    void calculate_delta_time();
private:
    std::unique_ptr<Window> m_window;
    Window::Properties m_windowProperties;

    std::chrono::steady_clock::time_point m_lastFrameBeginTime{ std::chrono::high_resolution_clock::now() };
    double m_deltaTime{ 0.0 };

    // Raw pointers as I want to really explicitly define the destroy order.
    struct
    {
        vk::Instance* instance{ nullptr };
        vk::Device* device{ nullptr };
        vk::RenderContext* context{ nullptr };
    }m_renderHandles{ };
};