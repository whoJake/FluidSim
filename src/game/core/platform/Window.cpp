#include "Window.h"

Window::Window(const Properties& properties) :
    m_properties(properties)
{ }

Window::Position Window::set_position(const Position& position)
{
    m_properties.position.x = position.x;
    m_properties.position.y = position.y;

    return m_properties.position;
}

Window::Extent Window::set_size(const Extent& extent) {
    if (m_properties.resizable) {
        m_properties.extent.width = extent.width;
        m_properties.extent.height = extent.height;
    }

    return m_properties.extent;
}

void Window::set_mode(const Mode& mode) {
    m_properties.mode = mode;
}

const Window::Extent& Window::get_extent() const {
    return m_properties.extent;
}

const Window::Mode& Window::get_window_mode() const {
    return m_properties.mode;
}

const Window::Properties& Window::get_properties() const {
    return m_properties;
}