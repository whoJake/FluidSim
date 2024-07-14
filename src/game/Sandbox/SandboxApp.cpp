#include "SandboxApp.h"

#include "core/Device.h"
#include "input/Input.h"
#include "system/timer.h"
#include "system/log.h"

#include "imgui.h"

#include "input/imgui/imgui_bindings.h"

SandboxApp::SandboxApp() :
    WindowedApplication("Windowed Application", { })
{ }

void SandboxApp::on_app_startup()
{
    ImGui::CreateContext();

    std::vector<VkPresentModeKHR> presentModes =
        { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };

    std::vector<VkSurfaceFormatKHR> surfaceFormats =
        { { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } };

    m_context = std::make_unique<vk::RenderContext>(
        get_device(),
        get_window().create_surface(get_instance()),
        get_window(),
        presentModes,
        surfaceFormats,
        vk::RenderTarget::no_depth_create_function);

    m_renderer = std::make_unique<ImageRenderer>(*m_context, &get_window(), &m_myguiContext);

    VkExtent3D extent{ 600, 400, 1 };

    m_image = std::make_unique<vk::Image>(
        m_context->get_device(),
        extent,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VK_SAMPLE_COUNT_1_BIT,
        1,
        1,
        VK_IMAGE_TILING_LINEAR
    );

    m_cpuImage = std::make_unique<Image>(
        static_cast<void*>(m_image->map()),
        extent.width,
        extent.height
    );

    m_camera = std::make_unique<Camera>(
        extent.width,
        extent.height,
        102
    );

    m_camera->position() = glm::vec3(0.f, 1.f, -3.f);
    m_camera->set_rotation(glm::vec3(45.f, 0.f, 0.f));

    m_blas = std::make_unique<Blas>(20);
}

void SandboxApp::on_app_shutdown()
{
    // Don't .dispose m_cpuImage as its data is owned by m_image
    m_image->unmap();
}

void SandboxApp::update()
{
    calculate_delta_time();
    get_window().process_events();

    m_myguiContext->begin_frame();

    // ImGui::ShowDemoWindow();

    ImGui::Begin("Settings");
    ImGui::SliderFloat3("Value", m_sunDirection, -1.f, 1.f);
    ImGui::InputInt("Bounces", &m_bounces);
    ImGui::End();

    parse_inputs();

    //ImGui::ShowDemoWindow();

    // debug frametime average
    {
        static u32 frame = 0;
        static u32 max = 50;

        static f32 accum = 0.f;

        if( frame == max )
        {
            f32 ft = (accum / max);
            f32 fps = 1.f / ft;
            // SYSLOG_PROFILE("{} fps average: {}fps", max, fps);
            get_window().set_title(std::format("{} : {}fps", get_application_name(), fps));
            frame = 0;
            accum = 0.f;
        }
        else
        {
            accum += f32_cast(m_deltaTime);
            frame++;
        }
    }

    static float currentRotation = 180.f;
    constexpr float distanceFromOrigin = 5.f;

#define ROTATE 1

    // Move camera
#if ROTATE
    m_camera->position() = glm::vec3(
        std::sinf(glm::radians(currentRotation)) * distanceFromOrigin,
        5.f,
        std::cosf(glm::radians(currentRotation)) * distanceFromOrigin);
#endif
    
    {
        std::string format = std::format("{},", static_cast<int>(currentRotation));
        format += " {}";
        
    #if 0
        sys::timer<sys::milliseconds> timer(&m_log, format.c_str());
    #elif 0
        sys::timer<sys::milliseconds> timer(&m_log, "Frametime {}");
    #endif

        for( size_t y = 0; y < m_camera->get_viewport_height(); y++ )
        {
            for( size_t x = 0; x < m_camera->get_viewport_width(); x++ )
            {
                mtl::ray ray = m_camera->get_pixel_ray(x, y);
                glm::vec3 outColor = m_blas->traverse(ray, { m_sunDirection[0], m_sunDirection[1], m_sunDirection[2] }, m_bounces);

                m_cpuImage->set_pixel(x, y, glm::vec4(outColor, 1.f));
            }
        }
    }

    // Rotate camera
    float degreesPerSecond = 60.f;
    currentRotation += degreesPerSecond * m_deltaTime;
    float temp;
    currentRotation = std::modf(currentRotation / 360.f, &temp) * 360.f;
    
#if ROTATE
    m_camera->rotate(glm::vec3(0.f, degreesPerSecond * m_deltaTime, 0.f));
#endif

    m_myguiContext->end_frame();

    m_renderer->render_image(m_image.get());
    Input::tick();
}

void SandboxApp::parse_inputs()
{
    if( Input::get_key_pressed(KeyCode::_1) )
        m_blas->mode = 1;
    if( Input::get_key_pressed(KeyCode::_2) )
        m_blas->mode = 2;
    if( Input::get_key_pressed(KeyCode::_3) )
        m_blas->mode = 3;
    if( Input::get_key_pressed(KeyCode::_4) )
        m_blas->mode = 4;

    glm::vec3 move{ };
    if( Input::get_key_down(KeyCode::A) )
        move.x -= 1;
    if( Input::get_key_down(KeyCode::D) )
        move.x += 1;
    if( Input::get_key_down(KeyCode::S) )
        move.z -= 1;
    if( Input::get_key_down(KeyCode::W) )
        move.z += 1;
    if( Input::get_key_down(KeyCode::Space) )
        move.y += 1;
    if( Input::get_key_down(KeyCode::LeftShift) )
        move.y -= 1;

    float speed = 10.f;
    move *= speed * m_deltaTime;

    glm::vec3 cameraForward = glm::normalize(m_camera->get_rotation() * glm::vec3(0.f, 0.f, 1.f));
    glm::vec3 cameraRight = glm::normalize(m_camera->get_rotation() * glm::vec3(1.f, 0.f, 0.f));

#if 0
    // Control camera with arrow keys
    float sensitivity = 90.f * static_cast<float>(m_deltaTime);
    glm::vec2 input{ };

    if( Input::get_key_down(KeyCode::Right) )
        input.x += 1.f;
    if( Input::get_key_down(KeyCode::Left) )
        input.x -= 1.f;
    if( Input::get_key_down(KeyCode::Up) )
        input.y += 1.f;
    if( Input::get_key_down(KeyCode::Down) )
        input.y -= 1.f;

    m_camera->rotate(glm::vec3(input.y * sensitivity, input.x * sensitivity, 0.f));
#else
    // Control camera with mouse
    if( Input::get_mouse_button_pressed(1) )
    {
        Input::set_cursor_lock_state(get_window(), CursorLockState::LOCKED);
    }
    else if( Input::get_mouse_button_released(1) )
    {
        Input::set_cursor_lock_state(get_window(), CursorLockState::NONE);
    }

    if( Input::get_mouse_button_down(1) )
    {
        float sensitivity = .2f;

        float mouseX = static_cast<float>(sensitivity * Input::get_mouse_move_horizontal());
        float mouseY = static_cast<float>(sensitivity * Input::get_mouse_move_vertical());

        m_camera->rotate(glm::vec3(mouseY, mouseX, 0.f));
    }
#endif

    if( Input::get_key_down(KeyCode::Enter) )
    {
        m_camera->position() = glm::vec3(0.f, 0.f, 0.f);
        m_camera->set_rotation(glm::vec3(0.f));
    }

    glm::vec3 translation = cameraRight * move.x + cameraForward * move.z + glm::vec3(0.f, 1.f, 0.f) * move.y;

    m_camera->position() += translation;
}

void SandboxApp::on_event(Event& e)
{
    mygui::dispatch_event(e);
    Input::register_event(e);

    EventDispatcher dispatch(e);
    dispatch.dispatch<WindowResizeEvent>(BIND_EVENT_FN(SandboxApp::on_window_resize));
}

bool SandboxApp::on_window_resize(WindowResizeEvent& e)
{
    if( m_camera )
    {
        if( e.get_width() <= 0.f
            || e.get_height() <= 0.f )
        {
            return false;
        }
    }

    return false;
}

void SandboxApp::calculate_delta_time()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto diff = static_cast<std::chrono::nanoseconds>(now - m_lastFrameBeginTime);
    m_deltaTime = diff.count() / 1e9;
    m_lastFrameBeginTime = now;
}