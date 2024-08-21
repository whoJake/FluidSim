#pragma once

#include "ApplicationBase.h"

#include "platform/Window.h"
#include "platform/events/Event.h"

// Vulkan forward declarations
struct VkPhysicalDeviceFeatures;
namespace vk
{
class Instance;
class Device;
}

class WindowedApplication: public ApplicationBase
{
public:
    WindowedApplication();
    WindowedApplication(const char* appName, const Window::Properties& properties);
    ~WindowedApplication();

    ExitFlags app_main() final;

    /// <summary>
    /// Ran after initialization of the app has occurred, but before the first call to update().
    /// </summary>
    virtual void on_app_startup() { };

    virtual void update() { };

    virtual void on_event(Event& e) { };

    Window& get_window();

    const char* get_application_name() const;
protected:
    virtual std::vector<const char*> request_instance_extensions() const;

    virtual std::vector<const char*> request_validation_layers() const;

    virtual std::vector<const char*> request_device_extensions() const;

    virtual VkPhysicalDeviceFeatures request_physical_device_feature_set() const;

    virtual std::optional<uint32_t> get_preferred_physical_device_index() const;

    vk::Instance& get_instance();

    vk::Device& get_device();
private:
    bool create_window();
    bool create_window(Window::Properties& properties);

    void initialize_handles();
private:
    const char* m_name;

    std::unique_ptr<Window> m_window;
    Window::Properties m_windowProperties;

    // Raw pointers as need to explicitly define the destroy order.
    struct
    {
        vk::Instance* instance{ nullptr };
        vk::Device* device{ nullptr };
    }m_handles{ };
};