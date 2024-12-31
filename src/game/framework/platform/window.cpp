#include "window.h"

namespace fw
{

window::window(const state& state) :
    m_state(state)
{ }

bool window::get_should_close() const
{
    return false;
}

void window::close()
{ }

void window::process_events()
{ }

bool window::is_headless() const
{
    return true;
}

void window::set_title(const std::string& title)
{
    m_state.title = title;
}

const glm::ivec2& window::set_position(const glm::ivec2& position)
{
    m_state.position = position;
    return m_state.position;
}

const glm::ivec2& window::set_size(const glm::ivec2& extent)
{
    m_state.extent = extent;
    return m_state.extent;
}

void window::set_mode(mode mode)
{
    m_state.mode = mode;
}

void window::set_cursor_lock_state(cursor_lock_state state)
{
    m_state.cursor_state = state;
}

cursor_lock_state window::get_cursor_lock_state() const
{
    return m_state.cursor_state;
}

glm::f64vec2 window::poll_mouse_pos() const
{
    return { 0.0, 0.0 };
}

const glm::ivec2& window::get_extent() const
{
    return m_state.extent;
}

const window::mode& window::get_window_mode() const
{
    return m_state.mode;
}

const window::state& window::get_state() const
{
    return m_state;
}

VkSurfaceKHR window::create_surface(vk::Instance& instance)
{
    return nullptr;
}

std::vector<const char*> window::get_required_surface_extensions() const
{
    return { };
}

#ifdef GFX_SUPPORTS_VULKAN
bool window::create_vulkan_surface(VkInstance instance, VkSurfaceKHR* surface)
{
    *surface = VK_NULL_HANDLE;
    return false;
}
#endif // GFX_SUPPORTS_VULKAN

} // fw