
#include "game.h"

#include "../platform/windows/window_glfw.h"
#include "core/Instance.h"
#include "core/PhysicalDevice.h"
#include "core/Device.h"
#include "rendering/RenderContext.h"

// DO NOT SUBMIT
#include "gfx_core/Driver.h"

#include "cdt/loaders/image_loaders.h"
#include "vulkan/vkdefines.h"

MAKEPARAM(no_vk_debug);

namespace fw
{

i32 game::app_main()
{
    while( !m_shouldClose && !m_window->get_should_close())
    {
        sys::moment updateStart = sys::now();
        auto timeSpent = updateStart - m_lastUpdateTime;
        m_lastUpdateTime = updateStart;

        bool success = update(std::chrono::duration_cast<std::chrono::nanoseconds>(timeSpent).count() / 1e9);
        if( !success )
        {
            return EXITCODE(game_exitcodes::failure_during_update);
        }

    }

    return EXITCODE(game_exitcodes::success);
}

bool game::on_startup()
{
    m_lastUpdateTime = sys::now();

    options startupOptions = get_startup_options();
    m_name = startupOptions.m_name;

    window::state initialWindowState = get_window_startup_state();

    if( gp_headless.get() )
    {
        SYSLOG_FATAL("Headless window is not supported.");
        return false;
    }
    else
    {
        m_window = std::make_unique<window_glfw>(initialWindowState);
    }

    if( !gp_no_graphics.get() )
    {
        bool init_success = initialise_graphics_handles(startupOptions);
        if( !init_success )
        {
            return false;
        }
    }

    return on_game_startup();
}

void game::on_shutdown()
{
    if( m_graphics.device )
        m_graphics.device->wait_idle();

    on_game_shutdown();

    if( m_graphics.context )
        m_graphics.context.reset();
    if( m_graphics.device )
        m_graphics.device.reset();
    if( m_graphics.instance )
        m_graphics.instance.reset();
}

bool game::initialise_graphics_handles(const options& options)
{
    std::vector<const char*> instanceExtensions = options.requested_vk_instance_extensions;
    std::vector<const char*> validationLayers = options.requested_vk_validation_layers;
    std::vector<const char*> deviceExtensions = options.requested_vk_device_extensions;

    if( !p_no_vk_debug.get() )
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");
        validationLayers.push_back("VK_LAYER_KHRONOS_synchronization2");
    }

    for( const char* windowRequiredExtension : m_window->get_required_surface_extensions() )
    {
        instanceExtensions.push_back(windowRequiredExtension);
    }

    if( !gp_headless.get() )
    {
        deviceExtensions.push_back("VK_KHR_swapchain");
    }

    m_graphics.instance = std::make_unique<vk::Instance>(
        m_name,
        "Framework Engine",
        VK_API_VERSION_1_3,
        instanceExtensions,
        validationLayers
    );

    m_graphics.instance->get_first_gpu().request_features(options.requested_vk_physical_device_features);

    m_graphics.device = std::make_unique<vk::Device>(
        m_graphics.instance->get_first_gpu(),
        m_window->create_surface(*m_graphics.instance),
        deviceExtensions
    );

    m_graphics.context = std::move(make_vk_render_context());
    return true;
}

std::unique_ptr<vk::RenderContext> game::make_vk_render_context()
{
    std::vector<VkPresentModeKHR> presentModePriority =
    {
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
    };

    std::vector<VkSurfaceFormatKHR> surfaceFormatPriority =
    {
        { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
    };

    return std::make_unique<vk::RenderContext>(
        *m_graphics.device,
        m_graphics.device->get_surface(),
        VkExtent2D{ u32_cast(m_window->get_extent().x), u32_cast(m_window->get_extent().y) },
        presentModePriority,
        surfaceFormatPriority,
        vk::RenderTarget::default_create_function
    );
}

void game::set_should_close()
{
    m_shouldClose = true;
}

window& game::get_window()
{
    return *m_window;
}

const window& game::get_window() const
{
    return *m_window;
}

game::graphics_handles& game::get_graphics_handles()
{
    return m_graphics;
}

bool game::update(f64 delta_time)
{
    return true;
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