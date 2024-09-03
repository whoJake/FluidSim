#pragma once
#include "Event.h"
#include <sstream>

class MouseEvent : public Event {
public:
    inline int get_button() const { return m_button; }

protected:
    MouseEvent(int button) :
        m_button(button) {}

    int m_button;
};

class MousePressedEvent : public MouseEvent {
public:
    MousePressedEvent(int button, int mods) :
        MouseEvent(button),
        m_modifier(mods) {}

    inline int get_modifiers() const { return m_modifier; }

    std::string to_str() const override {
        std::stringstream result;
        result << "MOUSE " << m_button << " PRESSED ( " << m_modifier << " )";
        return result.str();
    }

    EVENT_CLASS_TYPE(EVENT_MOUSE_PRESSED);
private:
    int m_modifier;
};

class MouseReleasedEvent : public MouseEvent {
public:
    MouseReleasedEvent(int button) :
        MouseEvent(button) {}

    std::string to_str() const override {
        std::stringstream result;
        result << "MOUSE " << m_button << " RELEASED";
        return result.str();
    }

    EVENT_CLASS_TYPE(EVENT_MOUSE_RELEASED);
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(double xOffset, double yOffset) :
        m_xOffset(xOffset),
        m_yOffset(yOffset) {}

    inline double get_offset_x() const { return m_xOffset; }
    inline double get_offset_y() const { return m_yOffset; }

    EVENT_CLASS_TYPE(EVENT_MOUSE_SCROLLED);
private:
    double m_xOffset, m_yOffset;
};

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(double xpos, double ypos) :
        m_xPos(xpos),
        m_yPos(ypos) {}

    inline double get_pos_x() const { return m_xPos; }
    inline double get_pos_y() const { return m_yPos; }

    std::string to_str() const override {
        std::stringstream result;
        result << "MOUSE MOVED ( " << m_xPos << ", " << m_yPos << " )";
        return result.str();
    }

    EVENT_CLASS_TYPE(EVENT_MOUSE_MOVED);
private:
    double m_xPos, m_yPos;
};