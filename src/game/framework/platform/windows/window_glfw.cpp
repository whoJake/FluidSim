#include "window_glfw.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

#include "../events/WindowEvent.h"
#include "../events/MouseEvent.h"
#include "../events/KeyEvent.h"

static KeyCode translate_key_code(int key) {
    static const std::unordered_map<int, KeyCode> lookup = {
        {GLFW_KEY_SPACE, KeyCode::Space},
        {GLFW_KEY_APOSTROPHE, KeyCode::Apostrophe},
        {GLFW_KEY_COMMA, KeyCode::Comma},
        {GLFW_KEY_MINUS, KeyCode::Minus},
        {GLFW_KEY_PERIOD, KeyCode::Period},
        {GLFW_KEY_SLASH, KeyCode::Slash},
        {GLFW_KEY_0, KeyCode::_0},
        {GLFW_KEY_1, KeyCode::_1},
        {GLFW_KEY_2, KeyCode::_2},
        {GLFW_KEY_3, KeyCode::_3},
        {GLFW_KEY_4, KeyCode::_4},
        {GLFW_KEY_5, KeyCode::_5},
        {GLFW_KEY_6, KeyCode::_6},
        {GLFW_KEY_7, KeyCode::_7},
        {GLFW_KEY_8, KeyCode::_8},
        {GLFW_KEY_9, KeyCode::_9},
        {GLFW_KEY_SEMICOLON, KeyCode::Semicolon},
        {GLFW_KEY_EQUAL, KeyCode::Equal},
        {GLFW_KEY_A, KeyCode::A},
        {GLFW_KEY_B, KeyCode::B},
        {GLFW_KEY_C, KeyCode::C},
        {GLFW_KEY_D, KeyCode::D},
        {GLFW_KEY_E, KeyCode::E},
        {GLFW_KEY_F, KeyCode::F},
        {GLFW_KEY_G, KeyCode::G},
        {GLFW_KEY_H, KeyCode::H},
        {GLFW_KEY_I, KeyCode::I},
        {GLFW_KEY_J, KeyCode::J},
        {GLFW_KEY_K, KeyCode::K},
        {GLFW_KEY_L, KeyCode::L},
        {GLFW_KEY_M, KeyCode::M},
        {GLFW_KEY_N, KeyCode::N},
        {GLFW_KEY_O, KeyCode::O},
        {GLFW_KEY_P, KeyCode::P},
        {GLFW_KEY_Q, KeyCode::Q},
        {GLFW_KEY_R, KeyCode::R},
        {GLFW_KEY_S, KeyCode::S},
        {GLFW_KEY_T, KeyCode::T},
        {GLFW_KEY_U, KeyCode::U},
        {GLFW_KEY_V, KeyCode::V},
        {GLFW_KEY_W, KeyCode::W},
        {GLFW_KEY_X, KeyCode::X},
        {GLFW_KEY_Y, KeyCode::Y},
        {GLFW_KEY_Z, KeyCode::Z},
        {GLFW_KEY_LEFT_BRACKET, KeyCode::LeftBracket},
        {GLFW_KEY_BACKSLASH, KeyCode::Backslash},
        {GLFW_KEY_RIGHT_BRACKET, KeyCode::RightBracket},
        {GLFW_KEY_GRAVE_ACCENT, KeyCode::GraveAccent},
        {GLFW_KEY_ESCAPE, KeyCode::Escape},
        {GLFW_KEY_ENTER, KeyCode::Enter},
        {GLFW_KEY_TAB, KeyCode::Tab},
        {GLFW_KEY_BACKSPACE, KeyCode::Backspace},
        {GLFW_KEY_INSERT, KeyCode::Insert},
        {GLFW_KEY_DELETE, KeyCode::DelKey},
        {GLFW_KEY_RIGHT, KeyCode::Right},
        {GLFW_KEY_LEFT, KeyCode::Left},
        {GLFW_KEY_DOWN, KeyCode::Down},
        {GLFW_KEY_UP, KeyCode::Up},
        {GLFW_KEY_PAGE_UP, KeyCode::PageUp},
        {GLFW_KEY_PAGE_DOWN, KeyCode::PageDown},
        {GLFW_KEY_HOME, KeyCode::Home},
        {GLFW_KEY_END, KeyCode::End},
        {GLFW_KEY_CAPS_LOCK, KeyCode::CapsLock},
        {GLFW_KEY_SCROLL_LOCK, KeyCode::ScrollLock},
        {GLFW_KEY_NUM_LOCK, KeyCode::NumLock},
        {GLFW_KEY_PRINT_SCREEN, KeyCode::PrintScreen},
        {GLFW_KEY_PAUSE, KeyCode::Pause},
        {GLFW_KEY_F1, KeyCode::F1},
        {GLFW_KEY_F2, KeyCode::F2},
        {GLFW_KEY_F3, KeyCode::F3},
        {GLFW_KEY_F4, KeyCode::F4},
        {GLFW_KEY_F5, KeyCode::F5},
        {GLFW_KEY_F6, KeyCode::F6},
        {GLFW_KEY_F7, KeyCode::F7},
        {GLFW_KEY_F8, KeyCode::F8},
        {GLFW_KEY_F9, KeyCode::F9},
        {GLFW_KEY_F10, KeyCode::F10},
        {GLFW_KEY_F11, KeyCode::F11},
        {GLFW_KEY_F12, KeyCode::F12},
        {GLFW_KEY_KP_0, KeyCode::KP_0},
        {GLFW_KEY_KP_1, KeyCode::KP_1},
        {GLFW_KEY_KP_2, KeyCode::KP_2},
        {GLFW_KEY_KP_3, KeyCode::KP_3},
        {GLFW_KEY_KP_4, KeyCode::KP_4},
        {GLFW_KEY_KP_5, KeyCode::KP_5},
        {GLFW_KEY_KP_6, KeyCode::KP_6},
        {GLFW_KEY_KP_7, KeyCode::KP_7},
        {GLFW_KEY_KP_8, KeyCode::KP_8},
        {GLFW_KEY_KP_9, KeyCode::KP_9},
        {GLFW_KEY_KP_DECIMAL, KeyCode::KP_Decimal},
        {GLFW_KEY_KP_DIVIDE, KeyCode::KP_Divide},
        {GLFW_KEY_KP_MULTIPLY, KeyCode::KP_Multiply},
        {GLFW_KEY_KP_SUBTRACT, KeyCode::KP_Subtract},
        {GLFW_KEY_KP_ADD, KeyCode::KP_Add},
        {GLFW_KEY_KP_ENTER, KeyCode::KP_Enter},
        {GLFW_KEY_KP_EQUAL, KeyCode::KP_Equal},
        {GLFW_KEY_LEFT_SHIFT, KeyCode::LeftShift},
        {GLFW_KEY_LEFT_CONTROL, KeyCode::LeftControl},
        {GLFW_KEY_LEFT_ALT, KeyCode::LeftAlt},
        {GLFW_KEY_RIGHT_SHIFT, KeyCode::RightShift},
        {GLFW_KEY_RIGHT_CONTROL, KeyCode::RightControl},
        {GLFW_KEY_RIGHT_ALT, KeyCode::RightAlt},
    };

    auto it = lookup.find(key);
    if (it == lookup.end())
        return KeyCode::Unknown;

    return it->second;
}

static void error_callback(int error, const char* msg)
{
    SYSLOG_ERROR("GLFW", "GLFW error code {}. {}", error, msg);
}

namespace fw
{

window_glfw::window_glfw(const state& state) :
    window(state),
    m_handle(nullptr)
{
    if( !glfwInit() )
    {
        // Make factory!!!!!!!
        return;
    }

    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, m_state.resizable);

    GLFWmonitor* monitor = m_state.mode == mode::windowed ? nullptr : glfwGetPrimaryMonitor();
    m_handle = glfwCreateWindow(m_state.extent.x, m_state.extent.y, m_state.title.c_str(), monitor, nullptr);
    set_mode(m_state.mode);

    glfwSetWindowUserPointer(m_handle, &m_state);
    setup_events();
}

window_glfw::~window_glfw()
{
    glfwTerminate();
}

void* window_glfw::get_native_handle() const
{
    return m_handle;
}

#ifdef GFX_SUPPORTS_VULKAN
bool window_glfw::create_vulkan_surface(VkInstance instance, VkSurfaceKHR* surface)
{
    VkResult result = glfwCreateWindowSurface(instance, m_handle, nullptr, surface);
    return result == VK_SUCCESS;
}
#endif // GFX_SUPPORTS_VULKAN

std::vector<const char*> window_glfw::get_required_surface_extensions() const
{
    const char** pExtensions;
    uint32_t extensionCount{ 0 };
    pExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<const char*> extensions(pExtensions, pExtensions + extensionCount);
    return extensions;
}

bool window_glfw::get_should_close() const
{
    return glfwWindowShouldClose(m_handle);
}

void window_glfw::close()
{
    glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
}

void window_glfw::process_events()
{
    glfwPollEvents();
}

void window_glfw::set_title(const std::string& title)
{
    glfwSetWindowTitle(m_handle, title.c_str());
}

const glm::ivec2& window_glfw::set_position(const glm::ivec2& position)
{
    glfwSetWindowPos(m_handle, position.x, position.y);

    int* x = nullptr;
    int* y = nullptr;
    glfwGetWindowPos(m_handle, x, y);

    if( x && y )
    {
        m_state.position.x = *x;
        m_state.position.y = *y;
    }

    return m_state.position;
}

const glm::ivec2& window_glfw::set_size(const glm::ivec2& extent)
{
    glfwSetWindowSize(m_handle, extent.x, extent.y);

    int* x = nullptr;
    int* y = nullptr;
    glfwGetWindowSize(m_handle, x, y);

    if( x && y )
    {
        m_state.extent.x = *x;
        m_state.extent.y = *y;
    }

    return m_state.extent;
}

void window_glfw::set_mode(mode mode)
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    int mX, mY, mWidth, mHeight;
    glfwGetMonitorWorkarea(monitor, &mX, &mY, &mWidth, &mHeight);
    int tlCentreX = mX + ((mWidth - m_state.extent.x) / 2);
    int tlCentreY = mY + ((mHeight - m_state.extent.y) / 2);

    switch( mode )
    {
    case mode::windowed:
    {
        glfwSetWindowAttrib(m_handle, GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowMonitor(m_handle, nullptr, tlCentreX, tlCentreY, m_state.extent.x, m_state.extent.y, GLFW_DONT_CARE);
        break;
    }
    case mode::fullscreen:
    {
        glfwSetWindowMonitor(m_handle, monitor, 0, 0, mWidth, mHeight, GLFW_DONT_CARE);
        break;
    }
    case mode::fullscreen_borderless:
    {
        set_mode(mode::windowed);
        glfwSetWindowAttrib(m_handle, GLFW_DECORATED, GLFW_FALSE);

        const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_handle, nullptr, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
        break;
    }
    case mode::fullscreen_stretched:
    {
        glfwSetWindowMonitor(m_handle, monitor, 0, 0, m_state.extent.x, m_state.extent.y, GLFW_DONT_CARE);
        break;
    }
    default:
        set_mode(mode::windowed);
        break;
    }
}

void window_glfw::set_cursor_lock_state(cursor_lock_state state)
{
    switch( state )
    {
    case cursor_lock_state::none:
        glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case cursor_lock_state::locked:
        glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if( glfwRawMouseMotionSupported() )
        {
            glfwSetInputMode(m_handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        break;
    case cursor_lock_state::constrainted:
        glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    }

    m_state.cursor_state = state;
}

glm::f64vec2 window_glfw::poll_mouse_pos() const
{
    glm::f64vec2 result{ };
    glfwGetCursorPos(m_handle, &result.x, &result.y);
    return result;
}

void window_glfw::setup_events() const
{
    // Window Events
    {
        glfwSetWindowCloseCallback(m_handle, [](GLFWwindow* owner)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                WindowClosedEvent e;
                data.eventfn(e);
            });

        glfwSetWindowPosCallback(m_handle, [](GLFWwindow* owner, i32 x, i32 y)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                data.position.x = x;
                data.position.y = y;

                WindowMovedEvent e(x, y);
                data.eventfn(e);
            });

        glfwSetFramebufferSizeCallback(m_handle, [](GLFWwindow* owner, i32 width, i32 height)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                data.extent.x = width;
                data.extent.y = height;

                WindowResizeEvent e(width, height);
                data.eventfn(e);
            });

        glfwSetWindowFocusCallback(m_handle, [](GLFWwindow* owner, i32 focused)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                if( focused )
                {
                    WindowFocusedEvent e;
                    data.eventfn(e);
                }
                else
                {
                    WindowUnFocusedEvent e;
                    data.eventfn(e);
                }
            });
    }

    // Key Events
    {
        glfwSetKeyCallback(m_handle, [](GLFWwindow* owner, i32 key, i32 scancode, i32 action, i32 mods)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                switch( action )
                {
                case GLFW_PRESS:
                {
                    KeyPressedEvent e(translate_key_code(key), 0);
                    data.eventfn(e);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent e(translate_key_code(key));
                    data.eventfn(e);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent e(translate_key_code(key), 1);
                    data.eventfn(e);
                    break;
                }
                }
            });

        glfwSetCharCallback(m_handle, [](GLFWwindow* owner, u32 key)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                KeyTypedEvent e(translate_key_code(key));
                data.eventfn(e);
            });
    }


    // Mouse Events
    {
        glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* owner, i32 button, i32 action, i32 mods)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                switch( action )
                {
                case GLFW_PRESS:
                {
                    MousePressedEvent e(button, mods);
                    data.eventfn(e);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseReleasedEvent e(button);
                    data.eventfn(e);
                    break;
                }
                }
            });

        glfwSetScrollCallback(m_handle, [](GLFWwindow* owner, f64 xoffset, f64 yoffset)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                MouseScrolledEvent e(xoffset, yoffset);
                data.eventfn(e);
            });

        glfwSetCursorPosCallback(m_handle, [](GLFWwindow* owner, f64 xpos, f64 ypos)
            {
                state data = *(state*)glfwGetWindowUserPointer(owner);

                if( data.cursor_state == cursor_lock_state::constrainted )
                {
                    if( xpos < 0.0 || xpos > f64_cast(data.extent.x)
                     || ypos < 0.0 || ypos > f64_cast(data.extent.y) )
                    {
                        xpos = glm::clamp(xpos, 0.0, f64_cast(data.extent.x));
                        ypos = glm::clamp(ypos, 0.0, f64_cast(data.extent.y));

                        // This doesn't queue another SetCursorPos event (that'll incorrectly call the callback next frame process_events)
                        // so its fine to manually set like this.
                        glfwSetCursorPos(owner, xpos, ypos);
                    }
                }

                MouseMovedEvent e(xpos, ypos);
                data.eventfn(e);
            });
    }
}

} // fw