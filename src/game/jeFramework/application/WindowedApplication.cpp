#include "WindowedApplication.h"

#include "platform/Window.h"
#include "implementations/WindowGlfw.h"

#include "core/Instance.h"
#include "core/PhysicalDevice.h"
#include "core/Device.h"
#include "core/RenderPass.h"
#include "core/PipelineLayout.h"
#include "core/Pipeline.h"

#include <sstream>

WindowedApplication::WindowedApplication() :
    m_windowProperties()
{ }

WindowedApplication::WindowedApplication(const Window::Properties& properties) :
    m_windowProperties(properties)
{ }

WindowedApplication::~WindowedApplication()
{ 
    // Not destroying in correct order but this isn't meant to be permenant
    get_render_context().get_device().wait_idle();

    delete m_renderHandles.context;
    delete m_renderHandles.device;
    delete m_renderHandles.instance;
}

ExitFlags WindowedApplication::app_main()
{
    if( !create_window() )
    {
        return ExitFlagBits::InitFailure;
    }

    if( !create_render_context() )
    {
        return ExitFlagBits::InitFailure;
    }

    on_app_startup();

    while( !m_window->get_should_close() )
    {
        calculate_delta_time();
        m_window->process_events();
        
        update(m_deltaTime);
    }

    return ExitFlagBits::Success;
}

vk::RenderContext& WindowedApplication::get_render_context()
{
    return *m_renderHandles.context;
}

Window& WindowedApplication::get_window()
{
    return *m_window;
}

bool WindowedApplication::create_window()
{
    m_windowProperties.eventfn = BIND_EVENT_FN(WindowedApplication::on_event);
    return create_window(m_windowProperties);
}

bool WindowedApplication::create_window(Window::Properties& properties)
{
    m_window = std::make_unique<WindowGlfw>(properties);

    return true;
}

bool WindowedApplication::create_render_context()
{
    std::vector<const char*> requestedInstanceExtensions = request_instance_extensions();
    std::vector<const char*> requestedValidationLayers;
    std::vector<const char*> requestedDeviceExtensions = request_device_extensions();
    requestedDeviceExtensions.push_back("VK_KHR_swapchain");

    if( Param_vulkan_debug_utils.get() )
    {
        requestedInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        requestedValidationLayers.push_back("VK_LAYER_KHRONOS_validation");
        for( const char* requestedLayer : request_validation_layers() )
        {
            requestedValidationLayers.push_back(requestedLayer);
        }
    }
    
    for( const char* requiredExt : m_window->get_required_surface_extensions() )
    {
        requestedInstanceExtensions.push_back(requiredExt);
    }

    m_renderHandles.instance = new vk::Instance(
        m_window->get_properties().title,
        "JCVE",
        VK_API_VERSION_1_3,
        requestedInstanceExtensions,
        requestedValidationLayers);

    vk::PhysicalDevice& activeGpu = m_renderHandles.instance->get_first_gpu();
    activeGpu.request_features(request_physical_device_feature_set());

    m_renderHandles.device = new vk::Device(
        activeGpu,
        m_window->create_surface(*m_renderHandles.instance),
        requestedDeviceExtensions);

    m_renderHandles.context = new vk::RenderContext(
        *m_renderHandles.device,
        m_renderHandles.device->get_surface(),
        *m_window,
        request_swapchain_present_mode(),
        request_swapchain_format());

    return true;
}

void WindowedApplication::calculate_delta_time()
{
    auto timeNow = std::chrono::high_resolution_clock::now();
    auto pointDifference = static_cast<std::chrono::nanoseconds>(timeNow - m_lastFrameBeginTime);
    m_deltaTime = pointDifference.count() / 1e9;
    m_lastFrameBeginTime = timeNow;
}

std::vector<const char*> WindowedApplication::request_instance_extensions() const
{
    return { };
}

std::vector<const char*> WindowedApplication::request_device_extensions() const
{
    return { };
}

std::vector<const char*> WindowedApplication::request_validation_layers() const
{
    return { };
}

VkPhysicalDeviceFeatures WindowedApplication::request_physical_device_feature_set() const
{
    return { };
}

std::vector<VkPresentModeKHR> WindowedApplication::request_swapchain_present_mode() const
{
    return { VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR };
}

std::vector<VkSurfaceFormatKHR> WindowedApplication::request_swapchain_format() const
{
    return { { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } };
}