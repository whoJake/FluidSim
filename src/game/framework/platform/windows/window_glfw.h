#pragma once

#include "../window.h"

struct GLFWwindow;

namespace fw
{

class window_glfw : public window
{
public:
    window_glfw(const state& state);
    virtual ~window_glfw();

    void* get_native_handle() const;
    bool get_should_close() const override;
    void close() override;
    void process_events() override;

    void set_title(const std::string& title) override;
    const glm::ivec2& set_position(const glm::ivec2& position) override;
    const glm::ivec2& set_size(const glm::ivec2& extent) override;
    void set_mode(mode mode) override;
    void set_cursor_lock_state(cursor_lock_state state) override;

    glm::f64vec2 poll_mouse_pos() const override;

#ifdef GFX_SUPPORTS_VULKAN
    bool create_vulkan_surface(VkInstance instance, VkSurfaceKHR* surface) override;
#endif // GFX_SUPPORTS_VULKAN

    std::vector<const char*> get_required_surface_extensions() const override;
private:
    void setup_events() const;
private:
    GLFWwindow* m_handle;
};

} // fw