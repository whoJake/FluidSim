#pragma once

#include "app.h"
#include "../platform/window.h"
#include "system/timer.h"
#include "core/Instance.h"

namespace vk
{

class PhysicalDevice;
class Device;
class RenderContext;

} // vk

namespace fw
{

enum class game_exitcodes
{
    success = app_exitcodes::success,
    failure_during_update = 10,
};

class game : public app
{
public:
    MAKEGPARAM(headless);
    MAKEGPARAM(no_graphics);

    struct options
    {
        const char* m_name{ nullptr };
        std::vector<const char*> requested_vk_instance_extensions{ };
        std::vector<const char*> requested_vk_validation_layers{ };
        std::vector<const char*> requested_vk_device_extensions{ };
        VkPhysicalDeviceFeatures requested_vk_physical_device_features{ };
    };

    struct graphics_handles
    {
        std::unique_ptr<vk::Instance> instance{ nullptr };
        std::unique_ptr<vk::Device> device{ nullptr };
        std::unique_ptr<vk::RenderContext> context{ nullptr };
    };

    game() = default;
    virtual ~game() = default;

    DELETE_MOVE(game);
    DELETE_COPY(game);

    i32 app_main() override;

    bool on_startup() override;
    void on_shutdown() override;

    void set_should_close();

    window& get_window();
    const window& get_window() const;

    graphics_handles& get_graphics_handles();

    virtual bool update(f64 delta_time);

    virtual bool on_game_startup();
    virtual void on_game_shutdown();

    virtual void on_event(Event& e);

    virtual options get_startup_options() = 0;
    virtual window::state get_window_startup_state() = 0;

    bool initialise_graphics_handles(const options& options);
    virtual std::unique_ptr<vk::RenderContext> make_vk_render_context();
private:
    const char* m_name{ nullptr };

    std::unique_ptr<window> m_window{ nullptr };
    graphics_handles m_graphics;

    sys::moment m_lastUpdateTime;
    bool m_shouldClose{ false };
};

} // fw