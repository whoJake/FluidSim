#include "imgui_bindings.h"

namespace mygui
{

constexpr ImGuiKey map_keycode(KeyCode code)
{
    switch( code )
    {
    case( KeyCode::Space ):
        return ImGuiKey_Space;
    case( KeyCode::Apostrophe ):
        return ImGuiKey_Apostrophe;
    case( KeyCode::Comma ):
        return ImGuiKey_Comma;
    case( KeyCode::Minus ):
        return ImGuiKey_Minus;
    case( KeyCode::Period ):
        return ImGuiKey_Period;
    case( KeyCode::Slash ):
        return ImGuiKey_Slash;
    case( KeyCode::_0 ):
        return ImGuiKey_0;
    case( KeyCode::_1 ):
        return ImGuiKey_1;
    case( KeyCode::_2 ):
        return ImGuiKey_2;
    case( KeyCode::_3 ):
        return ImGuiKey_3;
    case( KeyCode::_4 ):
        return ImGuiKey_4;
    case( KeyCode::_5 ):
        return ImGuiKey_5;
    case( KeyCode::_6 ):
        return ImGuiKey_6;
    case( KeyCode::_7 ):
        return ImGuiKey_7;
    case( KeyCode::_8 ):
        return ImGuiKey_8;
    case( KeyCode::_9 ):
        return ImGuiKey_9;
    case( KeyCode::Semicolon ):
        return ImGuiKey_Semicolon;
    case( KeyCode::Equal ):
        return ImGuiKey_Equal;
    case( KeyCode::A ):
        return ImGuiKey_A;
    case( KeyCode::B ):
        return ImGuiKey_B;
    case( KeyCode::C ):
        return ImGuiKey_C;
    case( KeyCode::D ):
        return ImGuiKey_D;
    case( KeyCode::E ):
        return ImGuiKey_E;
    case( KeyCode::F ):
        return ImGuiKey_F;
    case( KeyCode::G ):
        return ImGuiKey_G;
    case( KeyCode::H ):
        return ImGuiKey_H;
    case( KeyCode::I ):
        return ImGuiKey_I;
    case( KeyCode::J ):
        return ImGuiKey_J;
    case( KeyCode::K ):
        return ImGuiKey_K;
    case( KeyCode::L ):
        return ImGuiKey_L;
    case( KeyCode::M ):
        return ImGuiKey_M;
    case( KeyCode::N ):
        return ImGuiKey_N;
    case( KeyCode::O ):
        return ImGuiKey_O;
    case( KeyCode::P ):
        return ImGuiKey_P;
    case( KeyCode::Q ):
        return ImGuiKey_Q;
    case( KeyCode::R ):
        return ImGuiKey_R;
    case( KeyCode::S ):
        return ImGuiKey_S;
    case( KeyCode::T ):
        return ImGuiKey_T;
    case( KeyCode::U ):
        return ImGuiKey_U;
    case( KeyCode::V ):
        return ImGuiKey_V;
    case( KeyCode::W ):
        return ImGuiKey_W;
    case( KeyCode::X ):
        return ImGuiKey_X;
    case( KeyCode::Y ):
        return ImGuiKey_Y;
    case( KeyCode::Z ):
        return ImGuiKey_Z;
    case( KeyCode::LeftBracket ):
        return ImGuiKey_LeftBracket;
    case( KeyCode::Backslash ):
        return ImGuiKey_Backslash;
    case( KeyCode::RightBracket ):
        return ImGuiKey_RightBracket;
    case( KeyCode::GraveAccent ):
        return ImGuiKey_GraveAccent;
    case( KeyCode::Escape ):
        return ImGuiKey_Escape;
    case( KeyCode::Enter ):
        return ImGuiKey_Enter;
    case( KeyCode::Tab ):
        return ImGuiKey_Tab;
    case( KeyCode::Backspace ):
        return ImGuiKey_Backspace;
    case( KeyCode::LeftShift ):
        return ImGuiKey_LeftShift;
    case( KeyCode::LeftControl ):
        return ImGuiKey_LeftCtrl;
    case( KeyCode::LeftAlt ):
        return ImGuiKey_LeftAlt;
    case( KeyCode::RightShift ):
        return ImGuiKey_RightShift;
    case( KeyCode::RightControl ):
        return ImGuiKey_RightCtrl;
    case( KeyCode::RightAlt ):
        return ImGuiKey_RightAlt;
    case( KeyCode::Insert ):
        return ImGuiKey_Insert;
    case( KeyCode::DelKey ):
        return ImGuiKey_Delete;
    case( KeyCode::Right ):
        return ImGuiKey_RightArrow;
    case( KeyCode::Left ):
        return ImGuiKey_LeftArrow;
    case( KeyCode::Down ):
        return ImGuiKey_DownArrow;
    case( KeyCode::Up ):
        return ImGuiKey_UpArrow;
    case( KeyCode::PageUp ):
        return ImGuiKey_PageUp;
    case( KeyCode::PageDown ):
        return ImGuiKey_PageDown;
    case( KeyCode::Home ):
        return ImGuiKey_Home;
    case( KeyCode::End ):
        return ImGuiKey_End;
    case( KeyCode::Back ):
        return ImGuiKey_Backspace;
    case( KeyCode::CapsLock ):
        return ImGuiKey_CapsLock;
    case( KeyCode::ScrollLock ):
        return ImGuiKey_ScrollLock;
    case( KeyCode::NumLock ):
        return ImGuiKey_NumLock;
    case( KeyCode::PrintScreen ):
        return ImGuiKey_PrintScreen;
    case( KeyCode::Pause ):
        return ImGuiKey_Pause;
    case( KeyCode::F1 ):
        return ImGuiKey_F1;
    case( KeyCode::F2 ):
        return ImGuiKey_F2;
    case( KeyCode::F3 ):
        return ImGuiKey_F3;
    case( KeyCode::F4 ):
        return ImGuiKey_F4;
    case( KeyCode::F5 ):
        return ImGuiKey_F5;
    case( KeyCode::F6 ):
        return ImGuiKey_F6;
    case( KeyCode::F7 ):
        return ImGuiKey_F7;
    case( KeyCode::F8 ):
        return ImGuiKey_F8;
    case( KeyCode::F9 ):
        return ImGuiKey_F9;
    case( KeyCode::F10 ):
        return ImGuiKey_F10;
    case( KeyCode::F11 ):
        return ImGuiKey_F11;
    case( KeyCode::F12 ):
        return ImGuiKey_F12;
    case( KeyCode::KP_0 ):
        return ImGuiKey_Keypad0;
    case( KeyCode::KP_1 ):
        return ImGuiKey_Keypad1;
    case( KeyCode::KP_2 ):
        return ImGuiKey_Keypad2;
    case( KeyCode::KP_3 ):
        return ImGuiKey_Keypad3;
    case( KeyCode::KP_4 ):
        return ImGuiKey_Keypad4;
    case( KeyCode::KP_5 ):
        return ImGuiKey_Keypad5;
    case( KeyCode::KP_6 ):
        return ImGuiKey_Keypad6;
    case( KeyCode::KP_7 ):
        return ImGuiKey_Keypad7;
    case( KeyCode::KP_8 ):
        return ImGuiKey_Keypad8;
    case( KeyCode::KP_9 ):
        return ImGuiKey_Keypad9;
    case( KeyCode::KP_Decimal ):
        return ImGuiKey_KeypadDecimal;
    case( KeyCode::KP_Divide ):
        return ImGuiKey_KeypadDivide;
    case( KeyCode::KP_Multiply ):
        return ImGuiKey_KeypadMultiply;
    case( KeyCode::KP_Subtract ):
        return ImGuiKey_KeypadSubtract;
    case( KeyCode::KP_Add ):
        return ImGuiKey_KeypadAdd;
    case( KeyCode::KP_Enter ):
        return ImGuiKey_KeypadEnter;
    case( KeyCode::KP_Equal ):
        return ImGuiKey_KeypadEqual;
    case( KeyCode::Unknown ):
    default:
        return ImGuiKey_None;
    }
}

void dispatch_event(Event& event)
{
    EventDispatcher dispatch(event);

    dispatch.dispatch<KeyPressedEvent>(&::mygui::on_key_pressed);
    dispatch.dispatch<KeyReleasedEvent>(&::mygui::on_key_released);
    dispatch.dispatch<KeyTypedEvent>(&::mygui::on_key_typed);

    dispatch.dispatch<MousePressedEvent>(&::mygui::on_mouse_pressed);
    dispatch.dispatch<MouseReleasedEvent>(&::mygui::on_mouse_released);
    dispatch.dispatch<MouseScrolledEvent>(&::mygui::on_mouse_scrolled);
    dispatch.dispatch<MouseMovedEvent>(&::mygui::on_mouse_moved);

    dispatch.dispatch<WindowResizeEvent>(&::mygui::on_window_resize);
}

bool on_key_pressed(KeyPressedEvent& event)
{
    ImGuiKey key = map_keycode(event.get_key_code());
    ImGui::GetIO().KeysDown[key] = true;

    ImGui::GetIO().KeyCtrl = event.get_key_code() == KeyCode::LeftControl || event.get_key_code() == KeyCode::RightControl;
    ImGui::GetIO().KeyShift = event.get_key_code() == KeyCode::LeftShift || event.get_key_code() == KeyCode::RightShift;
    ImGui::GetIO().KeyAlt = event.get_key_code() == KeyCode::LeftAlt || event.get_key_code() == KeyCode::RightAlt;
    // ImGui::GetIO().KeySuper todo

    return ImGui::GetIO().WantCaptureKeyboard;
}

bool on_key_released(KeyReleasedEvent& event)
{
    ImGuiKey key = map_keycode(event.get_key_code());
    ImGui::GetIO().KeysDown[key] = false;

    ImGui::GetIO().KeyCtrl = event.get_key_code() == KeyCode::LeftControl || event.get_key_code() == KeyCode::RightControl;
    ImGui::GetIO().KeyShift = event.get_key_code() == KeyCode::LeftShift || event.get_key_code() == KeyCode::RightShift;
    ImGui::GetIO().KeyAlt = event.get_key_code() == KeyCode::LeftAlt || event.get_key_code() == KeyCode::RightAlt;
    // ImGui::GetIO().KeySuper todo

    return ImGui::GetIO().WantCaptureKeyboard;
}

bool on_key_typed(KeyTypedEvent& event)
{
    // todo
    return false;
}

bool on_mouse_pressed(MousePressedEvent& event)
{
    ImGui::GetIO().MouseDown[event.get_button()] = true;

    return ImGui::GetIO().WantCaptureMouse;
}

bool on_mouse_released(MouseReleasedEvent& event)
{
    ImGui::GetIO().MouseDown[event.get_button()] = false;

    return ImGui::GetIO().WantCaptureMouse;
}

bool on_mouse_scrolled(MouseScrolledEvent& event)
{
    ImGui::GetIO().MouseWheelH = f32_cast(event.get_offset_x());
    ImGui::GetIO().MouseWheel = f32_cast(event.get_offset_y());

    return ImGui::GetIO().WantCaptureMouse;
}

bool on_mouse_moved(MouseMovedEvent& event)
{
    ImGui::GetIO().MousePos = ImVec2(f32_cast(event.get_pos_x()), f32_cast(event.get_pos_y()));

    return ImGui::GetIO().WantCaptureMouse;
}

bool on_window_resize(WindowResizeEvent& event)
{
    ImGui::GetIO().DisplaySize = ImVec2(f32_cast(event.get_width()), f32_cast(event.get_height()));
    ImGui::GetIO().DisplayFramebufferScale = ImVec2(1.f, 1.f);

    return false;
}

} // mygui