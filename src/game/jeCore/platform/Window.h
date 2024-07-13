#pragma once
#include <string>
#include <functional>
#include "events/Event.h"

namespace vk
{
    class Instance;
} // vk
struct VkSurfaceKHR_T;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;

enum class CursorLockState
{
    NONE = 0,
    LOCKED
};

class Window {
public:
    struct Position {
        uint32_t x;
        uint32_t y;
    };

    struct Extent {
        uint32_t width;
        uint32_t height;
    };

    enum class Mode {
        Default,
        Fullscreen,
        FullscreenBorderless,
        FullscreenStretched,
        Windowed
    };

    enum class VSync {
        Default,
        On,
        Off
    };

    struct Properties {
        std::string title = "Windowed Application";
        Mode mode = Mode::Windowed;
        bool resizable = true;
        VSync vsync = VSync::Default;
        Position position = { 0, 0 };
        Extent extent = { 1600, 1200 };
        std::function<void(Event&)> eventfn = [](Event& e) {};
    };

public:
    Window(const Properties& properties);
    virtual ~Window() = default;

    virtual void* get_native_handle() const = 0;

    virtual VkSurfaceKHR create_surface(vk::Instance& instance) = 0;

    virtual std::vector<const char*> get_required_surface_extensions() const = 0;

    virtual bool get_should_close() const = 0;

    virtual void close() = 0;

    virtual void process_events() = 0;

    virtual void set_title(const std::string& title) = 0;

    virtual Position set_position(const Position& position);

    virtual Extent set_size(const Extent& extent);

    virtual void set_mode(const Mode& mode);

    virtual void set_cursor_lock_state(CursorLockState state) { }

    virtual CursorLockState get_cursor_lock_state() const { return CursorLockState::NONE; }

    virtual double poll_mouse_pos_x() const { return 0; }

    virtual double poll_mouse_pos_y() const { return 0; }

    const Extent& get_extent() const;

    const Mode& get_window_mode() const;

    const Properties& get_properties() const;
protected:
    Properties m_properties;
};
