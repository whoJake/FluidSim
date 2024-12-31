#pragma once
#include <functional>
#include "events/Event.h"

// This also brings in vulkan headers.
// Probably don't actually want it like this.
#include "gfx_core/gfxdefines.h"

namespace vk
{
    class Instance;
} // vk
struct VkSurfaceKHR_T;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;


namespace fw
{

enum class cursor_lock_state
{
    none = 0,
    locked,
    constrainted,
};

class window
{
public:
    enum class mode
    {
        windowed,
        fullscreen,
        fullscreen_borderless,
        fullscreen_stretched,
    };

    struct state
    {
        std::string title = "game";
        mode mode = mode::windowed;
        bool resizable = true;
        bool vsync = true;
        glm::ivec2 position = { 0, 0 };
        glm::ivec2 extent = { 1600, 1200 };

        cursor_lock_state cursor_state = cursor_lock_state::none;
        std::function<void(Event&)> eventfn = [](Event& e)
            { };
    };
protected:
    window(const state& state = { });
public:
    virtual ~window() = default;

    virtual void* get_native_handle() const = 0;

    virtual bool get_should_close() const;
    virtual void close();
    virtual void process_events();

    virtual bool is_headless() const;

    virtual void set_title(const std::string& title);
    virtual const glm::ivec2& set_position(const glm::ivec2& position);
    virtual const glm::ivec2& set_size(const glm::ivec2& extent);
    virtual void set_mode(mode mode);
    virtual void set_cursor_lock_state(cursor_lock_state state);

    cursor_lock_state get_cursor_lock_state() const;
    virtual glm::f64vec2 poll_mouse_pos() const;

    const glm::ivec2& get_extent() const;
    const mode& get_window_mode() const;
    const state& get_state() const;

    virtual VkSurfaceKHR create_surface(vk::Instance& instance);
    virtual std::vector<const char*> get_required_surface_extensions() const;

#ifdef GFX_SUPPORTS_VULKAN
    virtual bool create_vulkan_surface(VkInstance instance, VkSurfaceKHR* surface);
#endif // GFX_SUPPORTS_VULKAN
protected:
    state m_state;
};

} //fw
