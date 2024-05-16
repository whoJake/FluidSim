#pragma once
#include "Event.h"
#include <sstream>

enum class KeyCode : size_t {
    Unknown,
    Space,
    Apostrophe, /* ' */
    Comma,      /* , */
    Minus,      /* - */
    Period,     /* . */
    Slash,      /* / */
    _0,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    Semicolon, /* ; */
    Equal,     /* = */
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    LeftBracket,  /* [ */
    Backslash,    /* \ */
    RightBracket, /* ] */
    GraveAccent,  /* ` */
    Escape,
    Enter,
    Tab,
    Backspace,
    LeftShift,
    LeftControl,
    LeftAlt,
    RightShift,
    RightControl,
    RightAlt,
    Insert,
    DelKey,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    Back,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    KP_0,
    KP_1,
    KP_2,
    KP_3,
    KP_4,
    KP_5,
    KP_6,
    KP_7,
    KP_8,
    KP_9,
    KP_Decimal,
    KP_Divide,
    KP_Multiply,
    KP_Subtract,
    KP_Add,
    KP_Enter,
    KP_Equal,

    // Number of registered keys.
    KEYCODE_COUNT
};

inline std::string get_key_code_name(KeyCode keycode) {
    static const std::unordered_map<KeyCode, std::string> lookup = {
        {KeyCode::Space,        "EVENT_KEY_SPACE" },
        {KeyCode::Apostrophe,   "EVENT_KEY_APOSTROPHE" },
        {KeyCode::Comma,        "EVENT_KEY_COMMA" },
        {KeyCode::Minus,        "EVENT_KEY_MINUS" },
        {KeyCode::Period,       "EVENT_KEY_PERIOD" },
        {KeyCode::Slash,        "EVENT_KEY_SLASH" },
        {KeyCode::_0,           "EVENT_KEY_0" },
        {KeyCode::_1,           "EVENT_KEY_1" },
        {KeyCode::_2,           "EVENT_KEY_2" },
        {KeyCode::_3,           "EVENT_KEY_3" },
        {KeyCode::_4,           "EVENT_KEY_4" },
        {KeyCode::_5,           "EVENT_KEY_5" },
        {KeyCode::_6,           "EVENT_KEY_6" },
        {KeyCode::_7,           "EVENT_KEY_7" },
        {KeyCode::_8,           "EVENT_KEY_8" },
        {KeyCode::_9,           "EVENT_KEY_9" },
        {KeyCode::Semicolon,    "EVENT_KEY_SEMICOLON" },
        {KeyCode::Equal,        "EVENT_KEY_EQUAL" },
        {KeyCode::A,            "EVENT_KEY_A" },
        {KeyCode::B,            "EVENT_KEY_B" },
        {KeyCode::C,            "EVENT_KEY_C" },
        {KeyCode::D,            "EVENT_KEY_D" },
        {KeyCode::E,            "EVENT_KEY_E" },
        {KeyCode::F,            "EVENT_KEY_F" },
        {KeyCode::G,            "EVENT_KEY_G" },
        {KeyCode::H,            "EVENT_KEY_H" },
        {KeyCode::I,            "EVENT_KEY_I" },
        {KeyCode::J,            "EVENT_KEY_J" },
        {KeyCode::K,            "EVENT_KEY_K" },
        {KeyCode::L,            "EVENT_KEY_L" },
        {KeyCode::M,            "EVENT_KEY_M" },
        {KeyCode::N,            "EVENT_KEY_N" },
        {KeyCode::O,            "EVENT_KEY_O" },
        {KeyCode::P,            "EVENT_KEY_P" },
        {KeyCode::Q,            "EVENT_KEY_Q" },
        {KeyCode::R,            "EVENT_KEY_R" },
        {KeyCode::S,            "EVENT_KEY_S" },
        {KeyCode::T,            "EVENT_KEY_T" },
        {KeyCode::U,            "EVENT_KEY_U" },
        {KeyCode::V,            "EVENT_KEY_V" },
        {KeyCode::W,            "EVENT_KEY_W" },
        {KeyCode::X,            "EVENT_KEY_X" },
        {KeyCode::Y,            "EVENT_KEY_Y" },
        {KeyCode::Z,            "EVENT_KEY_Z" },
        {KeyCode::LeftBracket,  "EVENT_KEY_LEFT_BRACKET" },
        {KeyCode::Backslash,    "EVENT_KEY_BACKSLASH" },
        {KeyCode::RightBracket, "EVENT_KEY_RIGHT_BRACKET" },
        {KeyCode::GraveAccent,  "EVENT_KEY_GRAVE_ACCENT" },
        {KeyCode::Escape,       "EVENT_KEY_ESCAPE" },
        {KeyCode::Enter,        "EVENT_KEY_ENTER" },
        {KeyCode::Tab,          "EVENT_KEY_TAB" },
        {KeyCode::Backspace,    "EVENT_KEY_BACKSPACE" },
        {KeyCode::Insert,       "EVENT_KEY_INSERT" },
        {KeyCode::DelKey,       "EVENT_KEY_DELETE" },
        {KeyCode::Right,        "EVENT_KEY_RIGHT" },
        {KeyCode::Left,         "EVENT_KEY_LEFT" },
        {KeyCode::Down,         "EVENT_KEY_DOWN" },
        {KeyCode::Up,           "EVENT_KEY_UP" },
        {KeyCode::PageUp,       "EVENT_KEY_PAGE_UP" },
        {KeyCode::PageDown,     "EVENT_KEY_PAGE_DOWN" },
        {KeyCode::Home,         "EVENT_KEY_HOME" },
        {KeyCode::End,          "EVENT_KEY_END" },
        {KeyCode::CapsLock,     "EVENT_KEY_CAPS_LOCK" },
        {KeyCode::ScrollLock,   "EVENT_KEY_SCROLL_LOCK" },
        {KeyCode::NumLock,      "EVENT_KEY_NUM_LOCK" },
        {KeyCode::PrintScreen,  "EVENT_KEY_PRINT_SCREEN" },
        {KeyCode::Pause,        "EVENT_KEY_PAUSE" },
        {KeyCode::F1,           "EVENT_KEY_F1"},
        {KeyCode::F2,           "EVENT_KEY_F2"},
        {KeyCode::F3,           "EVENT_KEY_F3"},
        {KeyCode::F4,           "EVENT_KEY_F4"},
        {KeyCode::F5,           "EVENT_KEY_F5"},
        {KeyCode::F6,           "EVENT_KEY_F6"},
        {KeyCode::F7,           "EVENT_KEY_F7"},
        {KeyCode::F8,           "EVENT_KEY_F8"},
        {KeyCode::F9,           "EVENT_KEY_F9"},
        {KeyCode::F10,          "EVENT_KEY_F10" },
        {KeyCode::F11,          "EVENT_KEY_F11" },
        {KeyCode::F12,          "EVENT_KEY_F12" },
        {KeyCode::KP_0,         "EVENT_KEY_KP_0" },
        {KeyCode::KP_1,         "EVENT_KEY_KP_1" },
        {KeyCode::KP_2,         "EVENT_KEY_KP_2" },
        {KeyCode::KP_3,         "EVENT_KEY_KP_3"},
        {KeyCode::KP_4,         "EVENT_KEY_KP_4" },
        {KeyCode::KP_5,         "EVENT_KEY_KP_5"},
        {KeyCode::KP_6,         "EVENT_KEY_KP_6" },
        {KeyCode::KP_7,         "EVENT_KEY_KP_7" },
        {KeyCode::KP_8,         "EVENT_KEY_KP_8"},
        {KeyCode::KP_9,         "EVENT_KEY_KP_9" },
        {KeyCode::KP_Decimal,   "EVENT_KEY_KP_DECIMAL" },
        {KeyCode::KP_Divide,    "EVENT_KEY_KP_DIVIDE" },
        {KeyCode::KP_Multiply,  "EVENT_KEY_KP_MULTIPLY" },
        {KeyCode::KP_Subtract,  "EVENT_KEY_KP_SUBTRACT" },
        {KeyCode::KP_Add,       "EVENT_KEY_KP_ADD" },
        {KeyCode::KP_Enter,     "EVENT_KEY_KP_ENTER" },
        {KeyCode::KP_Equal,     "EVENT_KEY_KP_EQUAL" },
        {KeyCode::LeftShift,    "EVENT_KEY_LEFT_SHIFT" },
        {KeyCode::LeftControl,  "EVENT_KEY_LEFT_CONTROL" },
        {KeyCode::LeftAlt,      "EVENT_KEY_LEFT_ALT" },
        {KeyCode::RightShift,   "EVENT_KEY_RIGHT_SHIFT" },
        {KeyCode::RightControl, "EVENT_KEY_RIGHT_CONTROL" },
        {KeyCode::RightAlt,     "EVENT_KEY_RIGHT_ALT" }
    };

    auto it = lookup.find(keycode);
    if (it == lookup.end())
        return "EVENT_KEY_UNKNOWN";
    return it->second;
}

class KeyEvent : public Event {
public:
    inline KeyCode get_key_code() const { return m_keyCode; }
protected:
    KeyEvent(KeyCode keycode) : 
        m_keyCode(keycode) 
    { }
    KeyCode m_keyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(KeyCode keycode, int repeatCount) :
        KeyEvent(keycode),
        m_repeatCount(repeatCount)
    { }

    inline int get_repeat_count() const { return m_repeatCount; }

    std::string to_str() const override {
        std::stringstream result;
        result << get_key_code_name(m_keyCode) << " x" << m_repeatCount << " PRESSED";
        return result.str();
    }

    EVENT_CLASS_TYPE(EVENT_KEY_PRESSED);
private:
    int m_repeatCount;
};

class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(KeyCode keycode) :
        KeyEvent(keycode)
    { }

    EVENT_CLASS_TYPE(EVENT_KEY_RELEASED);

    std::string to_str() const override {
        std::stringstream result;
        result << get_key_code_name(m_keyCode) << " RELEASED";
        return result.str();
    }
};

class KeyTypedEvent : public KeyEvent {
public:
    KeyTypedEvent(KeyCode keycode) :
        KeyEvent(keycode)
    { }

    std::string to_str() const override {
        std::stringstream result;
        result << get_key_code_name(m_keyCode) << " TYPED";
        return result.str();
    }

    EVENT_CLASS_TYPE(EVENT_KEY_TYPED);
};
