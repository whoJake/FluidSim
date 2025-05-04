
#include "game.h"

#include "../platform/windows/window_glfw.h"
#include "gfx_core/Driver.h"
#include "gfx_fw/render_interface.h"
#include "basic/Time.h"

namespace fw
{

i32 game::app_main()
{
    setup_startup_graph(scaffold::add_startup_node(scaffold_startup_node([]() -> void
        {
            // Nothing to do in base startup :/
        })));

    setup_update_graph(scaffold::add_update_node(scaffold_startup_node([]() -> void
        {
            Time::update();
        })));

    setup_shutdown_graph(scaffold::add_shutdown_node(scaffold_shutdown_node([]() -> void
    {
        // Nothing to do in base shutdown
    })));

    scaffold::startup();
    return EXIT_SUCCESS;
}

bool game::on_startup()
{
    m_lastUpdateTime = sys::now();

    window::state initialWindowState = get_window_startup_state();
    m_window = std::make_unique<window_glfw>(initialWindowState);

    if( gfx::driver::initialise(
            gfx::DRIVER_MODE_VULKAN,
            std::bind(
                &fw::window::create_vulkan_surface,
                reinterpret_cast<window_glfw*>(m_window.get()),
                std::placeholders::_1,
                std::placeholders::_2)) )
    {
        return false;
    }

    GFX_CALL(dump_info);

    gfx::fw::render_interface::set_target_swapchain_extents(initialWindowState.extent.x, initialWindowState.extent.y);
    gfx::fw::render_interface::initialise();

    return on_game_startup();
}

void game::on_shutdown()
{
    gfx::driver::wait_idle();
    on_game_shutdown();
    gfx::fw::render_interface::shutdown();
    gfx::driver::shutdown();
    m_window->close();
}

void game::setup_startup_graph(scaffold_startup_node& parent)
{ }

void game::setup_update_graph(scaffold_update_node& parent)
{ }

void game::setup_shutdown_graph(scaffold_shutdown_node& parent)
{ }

void game::set_should_close()
{
    m_shouldClose = true;
    scaffold::set_should_stop();
}

window& game::get_window()
{
    return *m_window;
}

const window& game::get_window() const
{
    return *m_window;
}

bool game::on_game_startup()
{
    return true;
}

void game::on_game_shutdown()
{ }

void game::on_event(Event& e)
{ }

} // fw