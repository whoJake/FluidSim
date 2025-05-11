#include "Input.h"

void Input::set_cursor_lock_state(fw::window& window, fw::cursor_lock_state state)
{
    if( get_instance()->m_primaryCursorLockState == state )
    {
        return;
    }
    get_instance()->m_primaryCursorLockState = state;
    window.set_cursor_lock_state(state);

    get_instance()->m_previousMouseX = window.poll_mouse_pos().x;
    get_instance()->m_previousMouseY = window.poll_mouse_pos().y;
}

bool Input::get_key_pressed(KeyCode key)
{
    return get_key_state(key) == KeyState::PRESSED;
}

bool Input::get_key_down(KeyCode key)
{
    return get_key_state(key) >= KeyState::PRESSED;
}

bool Input::get_key_released(KeyCode key)
{
    return get_key_state(key) == KeyState::RELEASED;
}

bool Input::get_mouse_button_pressed(uint8_t button)
{
    if( button >= INPUT_MAX_MOUSE_BUTTONS )
    {
        QUITFMT("Polling for mouse button that is past the max supported mouse buttons.");
    }

    return get_mouse_key_state(button) == KeyState::PRESSED;
}

bool Input::get_mouse_button_down(uint8_t button)
{
    if( button >= INPUT_MAX_MOUSE_BUTTONS )
    {
        QUITFMT("Polling for mouse button that is past the max supported mouse buttons.");
    }

    return get_mouse_key_state(button) >= KeyState::PRESSED;
}

bool Input::get_mouse_button_released(uint8_t button)
{
    if( button >= INPUT_MAX_MOUSE_BUTTONS )
    {
        QUITFMT("Polling for mouse button that is past the max supported mouse buttons.");
    }

    return get_mouse_key_state(button) == KeyState::RELEASED;
}

double Input::get_mouse_move_horizontal()
{
    return get_instance()->m_currentMouseX - get_instance()->m_previousMouseX;
}

double Input::get_mouse_move_vertical()
{
    return get_instance()->m_currentMouseY - get_instance()->m_previousMouseY;
}

fw::cursor_lock_state Input::get_cursor_lock_state()
{
    return get_instance()->m_primaryCursorLockState;
}

double Input::get_mouse_scroll_horizontal()
{
    return get_instance()->m_scrollXOffset;
}

double Input::get_mouse_scroll_vertical()
{
    return get_instance()->m_scrollYOffset;
}

void Input::tick()
{
    get_instance()->m_previousMouseX = get_instance()->m_currentMouseX;
    get_instance()->m_previousMouseY = get_instance()->m_currentMouseY;
    get_instance()->m_scrollXOffset = 0.f;
    get_instance()->m_scrollYOffset = 0.f;

    for( KeyState& state : get_instance()->m_keyStates )
    {
        switch( state )
        {
        case KeyState::RELEASED:
            state = KeyState::NONE;
            break;
        case KeyState::PRESSED:
            state = KeyState::HELD;
            break;
        default:
            break;
        }
    }

    for( KeyState& state : get_instance()->m_mouseKeyStates )
    {
        switch( state )
        {
        case KeyState::RELEASED:
            state = KeyState::NONE;
            break;
        case KeyState::PRESSED:
            state = KeyState::HELD;
            break;
        default:
            break;
        }
    }
}

bool Input::register_event(Event& event)
{
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<KeyPressedEvent>(Input::register_key_press_event);
    dispatcher.dispatch<KeyReleasedEvent>(Input::register_key_release_event);
    dispatcher.dispatch<MousePressedEvent>(Input::register_mouse_press_event);
    dispatcher.dispatch<MouseReleasedEvent>(Input::register_mouse_release_event);
    dispatcher.dispatch<MouseScrolledEvent>(Input::register_mouse_scroll_event);
    dispatcher.dispatch<MouseMovedEvent>(Input::register_mouse_move_event);

    return false;
}

double Input::get_mouse_x()
{
    return get_instance()->m_currentMouseX;
}

double Input::get_mouse_y()
{
    return get_instance()->m_currentMouseY;
}

KeyState& Input::get_key_state(KeyCode code)
{
    return get_instance()->m_keyStates[static_cast<size_t>(code)];
}

KeyState& Input::get_mouse_key_state(uint8_t button)
{
    return get_instance()->m_mouseKeyStates[static_cast<size_t>(button)];
}

bool Input::register_key_press_event(KeyPressedEvent& event)
{
    KeyState& currentState = get_key_state(event.get_key_code());
    if( currentState != KeyState::HELD )
    {
        currentState = KeyState::PRESSED;
    }

    return false;
}

bool Input::register_key_release_event(KeyReleasedEvent& event)
{
    get_key_state(event.get_key_code()) = KeyState::RELEASED;
    return false;
}

bool Input::register_mouse_press_event(MousePressedEvent& event)
{
    if( event.get_button() >= INPUT_MAX_MOUSE_BUTTONS )
    {
        QUITFMT("Trying to register mouse button that is past the max supported mouse buttons.");
    }

    KeyState& currentState = get_mouse_key_state(static_cast<uint8_t>(event.get_button()));
    if( currentState != KeyState::HELD )
    {
        currentState = KeyState::PRESSED;
    }

    return false;
}

bool Input::register_mouse_release_event(MouseReleasedEvent& event)
{
    if( event.get_button() >= INPUT_MAX_MOUSE_BUTTONS )
    {
        QUITFMT("Trying to register mouse button that is past the max supported mouse buttons.");
    }

    get_mouse_key_state(static_cast<uint8_t>(event.get_button())) = KeyState::RELEASED;

    return false;
}

bool Input::register_mouse_scroll_event(MouseScrolledEvent& event)
{
    get_instance()->m_scrollXOffset = event.get_offset_x();
    get_instance()->m_scrollYOffset = event.get_offset_y();
    return false;
}

bool Input::register_mouse_move_event(MouseMovedEvent& event)
{
    get_instance()->m_currentMouseX = event.get_pos_x();
    get_instance()->m_currentMouseY = event.get_pos_y();
    return false;
}

Input::Input()
{ }

Input* Input::get_instance()
{
    if( !s_instance )
    {
        s_instance = new Input();
    }

    return s_instance;
}

void Input::shutdown()
{
    delete s_instance;
}