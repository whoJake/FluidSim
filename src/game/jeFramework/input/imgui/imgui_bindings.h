#pragma once

#include "platform/events/Event.h"
#include "platform/events/WindowEvent.h"
#include "platform/events/MouseEvent.h"
#include "platform/events/KeyEvent.h"
#include "imgui.h"

namespace mygui
{

inline constexpr ImGuiKey map_keycode(KeyCode code);

inline void dispatch_event(Event& event);

inline bool on_key_pressed(KeyPressedEvent& event);

inline bool on_key_released(KeyReleasedEvent& event);

inline bool on_key_typed(KeyTypedEvent& event);

inline bool on_mouse_pressed(MousePressedEvent& event);

inline bool on_mouse_released(MouseReleasedEvent& event);

inline bool on_mouse_scrolled(MouseScrolledEvent& event);

inline bool on_mouse_moved(MouseMovedEvent& event);

inline bool on_window_resize(WindowResizeEvent& event);

} // mygui

#ifndef INC_IMGUI_BINDINGS_INL
#define INC_IMGUI_BINDINGS_INL
#include "imgui_bindings.inl"
#endif