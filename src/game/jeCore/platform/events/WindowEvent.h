#pragma once
#include "Event.h"
#include <sstream>

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(int width, int height) :
        m_width(width),
        m_height(height) {}

    inline int get_width() const { return m_width; }
    inline int get_height() const { return m_height; }

    std::string to_str() const override {
        std::stringstream result;
        result << "( " << m_width << ", " << m_height << " ) RESIZE";
        return result.str();
    }

    EVENT_CLASS_TYPE(EVENT_WINDOW_RESIZE);
private:
    int m_width, m_height;
};

class WindowMovedEvent: public Event {
public:
    WindowMovedEvent(int x, int y) :
        m_xPos(x),
        m_yPos(y) {}

    inline int get_pos_x() const { return m_xPos; }
    inline int get_pos_y() const { return m_yPos; }

    std::string to_str() const override {
        std::stringstream result;
        result << "( " << m_xPos << ", " << m_yPos << " ) MOVED";
        return result.str();
    }

    EVENT_CLASS_TYPE(EVENT_WINDOW_MOVED);
private:
    int m_xPos, m_yPos;
};

class WindowClosedEvent : public Event {
public:
    WindowClosedEvent() {}

    EVENT_CLASS_TYPE(EVENT_WINDOW_CLOSED);
};

class WindowFocusedEvent : public Event {
public:
    WindowFocusedEvent() {}

    EVENT_CLASS_TYPE(EVENT_WINDOW_FOCUSED);
};

class WindowUnFocusedEvent : public Event {
public:
    WindowUnFocusedEvent() {}

    EVENT_CLASS_TYPE(EVENT_WINDOW_UNFOCUSED);
};
