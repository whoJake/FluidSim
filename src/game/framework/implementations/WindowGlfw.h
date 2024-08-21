#pragma once

#include "platform/Window.h"

struct GLFWwindow;

class WindowGlfw : public Window {
public:
    WindowGlfw(const Properties& properties);
    virtual ~WindowGlfw();

    void* get_native_handle() const;

    VkSurfaceKHR create_surface(vk::Instance& instance) override;

    std::vector<const char*> get_required_surface_extensions() const override;

    bool get_should_close() const override;

    void close() override;

    void process_events() override;

    void set_title(const std::string& title) override;

    Position set_position(const Position& position) override;

    Extent set_size(const Extent& extent) override;

    void set_mode(const Mode& mode) override;

    void set_cursor_lock_state(CursorLockState state) override;

    CursorLockState get_cursor_lock_state() const override;

    double poll_mouse_pos_x() const override;

    double poll_mouse_pos_y() const override;
private:
    void setup_events() const;
private:
    GLFWwindow* m_handle;
};
