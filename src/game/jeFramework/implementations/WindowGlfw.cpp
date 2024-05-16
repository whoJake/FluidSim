#include "WindowGlfw.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

#include "platform/events/WindowEvent.h"
#include "platform/events/MouseEvent.h"
#include "platform/events/KeyEvent.h"

#include "core/Instance.h"

inline KeyCode translate_key_code(int key) {
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
    if( g_singleThreadedLog )
    {
        JCLOG_ERROR(*g_singleThreadedLog, "GLFW", "GLFW error code {}. {}", error, msg);
    }
}

WindowGlfw::WindowGlfw(const Properties& properties):
    Window(properties),
    m_handle(nullptr)
{
    if( !glfwInit() )
    {
        // Make factory!!!!!!!
        return;
    }

    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, properties.resizable);

    GLFWmonitor* monitor = m_properties.mode == Mode::Windowed ? nullptr : glfwGetPrimaryMonitor();
    m_handle = glfwCreateWindow(m_properties.extent.width, m_properties.extent.height, m_properties.title.c_str(), monitor, nullptr);
    set_mode(properties.mode);

    glfwSetWindowUserPointer(m_handle, &m_properties);
    setup_events();
}

WindowGlfw::~WindowGlfw()
{
    glfwTerminate();
}

void* WindowGlfw::get_native_handle() const
{
    return m_handle;
}

VkSurfaceKHR WindowGlfw::create_surface(vk::Instance& instance)
{
    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    VkResult result = glfwCreateWindowSurface(instance.get_handle(), m_handle, nullptr, &surface);
    VK_CHECK(result, "Failed to create VkSurface on GLFW window.");

    return surface;
}

std::vector<const char*> WindowGlfw::get_required_surface_extensions() const
{
    const char** pExtensions;
    uint32_t extensionCount{ 0 };
    pExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<const char*> extensions(pExtensions, pExtensions + extensionCount);
    return extensions;
}

bool WindowGlfw::get_should_close() const
{
    return glfwWindowShouldClose(m_handle);
}

void WindowGlfw::close()
{
    glfwSetWindowShouldClose(m_handle, GLFW_TRUE);
}

void WindowGlfw::process_events()
{
    glfwPollEvents();
}

void WindowGlfw::set_title(const std::string& title)
{
    glfwSetWindowTitle(m_handle, title.c_str());
}

Window::Position WindowGlfw::set_position(const Window::Position& position)
{
    glfwSetWindowPos(m_handle, position.x, position.y);
    int* x = nullptr;
    int* y = nullptr;
    glfwGetWindowPos(m_handle, x, y);

    if( x && y )
    {
        m_properties.position.x = *x;
        m_properties.position.y = *y;
    }

    return m_properties.position;
}

Window::Extent WindowGlfw::set_size(const Window::Extent& extent)
{
    glfwSetWindowSize(m_handle, extent.width, extent.height);
    int* x = nullptr;
    int* y = nullptr;
    glfwGetWindowSize(m_handle, x, y);

    if( x && y )
    {
        m_properties.extent.width = *x;
        m_properties.extent.height = *y;
    }

    return m_properties.extent;
}

void WindowGlfw::set_mode(const Window::Mode& mode)
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    int mX, mY, mWidth, mHeight;
    glfwGetMonitorWorkarea(monitor, &mX, &mY, &mWidth, &mHeight);
    int tlCentreX = mX + ((mWidth - m_properties.extent.width) / 2);
    int tlCentreY = mY + ((mHeight - m_properties.extent.height) / 2);

    switch( mode ) {
        case Mode::Windowed:
        {
            glfwSetWindowAttrib(m_handle, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowMonitor(m_handle, nullptr, tlCentreX, tlCentreY, m_properties.extent.width, m_properties.extent.height, GLFW_DONT_CARE);
            break;
        }
        case Mode::Fullscreen:
        {
            glfwSetWindowMonitor(m_handle, monitor, 0, 0, mWidth, mHeight, GLFW_DONT_CARE);
            break;
        }
        case Mode::FullscreenBorderless:
        {
            set_mode(Mode::Windowed);
            glfwSetWindowAttrib(m_handle, GLFW_DECORATED, GLFW_FALSE);

            const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(m_handle, nullptr, 0, 0, vidmode->width, vidmode->height, vidmode->refreshRate);
            break;
        }
        case Mode::FullscreenStretched:
        {
            glfwSetWindowMonitor(m_handle, monitor, 0, 0, m_properties.extent.width, m_properties.extent.height, GLFW_DONT_CARE);
            break;
        }
        case Mode::Default:
        {
            set_mode(Mode::Windowed);
            break;
        }
    }
}

void WindowGlfw::set_cursor_lock_state(CursorLockState state)
{
    switch( state )
    {
    case CursorLockState::NONE:
        glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case CursorLockState::LOCKED:
        glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if( glfwRawMouseMotionSupported() )
        {
            glfwSetInputMode(m_handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        break;
    }
}

CursorLockState WindowGlfw::get_cursor_lock_state() const
{
    switch( glfwGetInputMode(m_handle, GLFW_CURSOR) )
    {
    case GLFW_CURSOR_NORMAL:
        return CursorLockState::NONE;
    case GLFW_CURSOR_DISABLED:
        return CursorLockState::LOCKED;
    default:
        return CursorLockState::NONE;
    }
}

double WindowGlfw::poll_mouse_pos_x() const
{
    double x, y;
    glfwGetCursorPos(m_handle, &x, &y);
    return x;
}

double WindowGlfw::poll_mouse_pos_y() const
{
    double x, y;
    glfwGetCursorPos(m_handle, &x, &y);
    return y;
}

void WindowGlfw::setup_events() const
{
    // Window Events
    {
        glfwSetWindowCloseCallback(m_handle, [](GLFWwindow* owner){
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            WindowClosedEvent e;
            data.eventfn(e);
        });

        glfwSetWindowPosCallback(m_handle, [](GLFWwindow* owner, int x, int y){ 
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            data.position.x = x;
            data.position.y = y;

            WindowMovedEvent e(x, y);
            data.eventfn(e);
        });

        glfwSetFramebufferSizeCallback(m_handle, [](GLFWwindow* owner, int width, int height) {
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            data.extent.width = width;
            data.extent.height = height;

            WindowResizeEvent e(width, height);
            data.eventfn(e);
        });

        glfwSetWindowFocusCallback(m_handle, [](GLFWwindow* owner, int focused) {
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            if( focused ) {
                WindowFocusedEvent e;
                data.eventfn(e);
            }
            else {
                WindowUnFocusedEvent e;
                data.eventfn(e);
            }
        });
    }

    // Key Events
    {
        glfwSetKeyCallback(m_handle, [](GLFWwindow* owner, int key, int scancode, int action, int mods) {
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            switch( action ) {
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

        glfwSetCharCallback(m_handle, [](GLFWwindow* owner, unsigned int key) {
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            KeyTypedEvent e(translate_key_code(key));
            data.eventfn(e);
        });
    }


    // Mouse Events
    {
        glfwSetMouseButtonCallback(m_handle, [](GLFWwindow* owner, int button, int action, int mods) {
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            switch( action ) {
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

        glfwSetScrollCallback(m_handle, [](GLFWwindow* owner, double xoffset, double yoffset) {
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            MouseScrolledEvent e(xoffset, yoffset);
            data.eventfn(e);
        });

        glfwSetCursorPosCallback(m_handle, [](GLFWwindow* owner, double xpos, double ypos) {
            Properties data = *(Properties*)glfwGetWindowUserPointer(owner);

            MouseMovedEvent e(xpos, ypos);
            data.eventfn(e);
        });
    }
}
