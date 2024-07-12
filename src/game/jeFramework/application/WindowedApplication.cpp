#include "WindowedApplication.h"

#include "platform/Window.h"
#include "implementations/WindowGlfw.h"

#include "core/Instance.h"
#include "core/PhysicalDevice.h"
#include "core/Device.h"

#include <sstream>

WindowedApplication::WindowedApplication() :
    m_name("DEFAULT NAME"),
    m_windowProperties()
{ }

WindowedApplication::WindowedApplication(const char* appName, const Window::Properties& properties) :
    m_name(appName),
    m_windowProperties(properties)
{ }

WindowedApplication::~WindowedApplication()
{
    if( m_handles.device )
    {
        m_handles.device->wait_idle();
        delete m_handles.device;
    }

    delete m_handles.instance;
}

ExitFlags WindowedApplication::app_main()
{
    if( !create_window() )
    {
        return ExitFlagBits::InitFailure;
    }

    initialize_handles();
    on_app_startup();

    while( !m_window->get_should_close() )
    {
        m_window->process_events();
        update();
    }

    return ExitFlagBits::Success;
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

void WindowedApplication::initialize_handles()
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

    m_handles.instance = new vk::Instance(
        m_window->get_properties().title,
        m_name,
        VK_API_VERSION_1_3,
        requestedInstanceExtensions,
        requestedValidationLayers);

    vk::PhysicalDevice& activeGpu = m_handles.instance->get_first_gpu();
    activeGpu.request_features(request_physical_device_feature_set());

    m_handles.device = new vk::Device(
        activeGpu,
        m_window->create_surface(*m_handles.instance),
        requestedDeviceExtensions);

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

std::optional<uint32_t> WindowedApplication::get_preferred_physical_device_index() const
{
    return std::nullopt;
}

vk::Instance& WindowedApplication::get_instance()
{
    return *m_handles.instance;
}

vk::Device& WindowedApplication::get_device()
{
    return *m_handles.device;
}