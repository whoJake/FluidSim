#include "ImGuiContext.h"

#include "imgui.h"
#include "glfw3.h"

#include "backends/imgui_impl_vulkan.h"
#include "gfx_core/driver.h"
#include "gfx_fw/render_interface.h"

namespace mygui
{

Context::Context(fw::window* window) :
    m_window(window),
    m_pool(),
    m_desc()
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    viewport->PlatformHandle = window->get_native_handle();
    // viewport->PlatformHandleRaw = glfwGetWin32Window(glfwWindow->get_native_handle());

    dt::vector<gfx::descriptor_slot_desc> image_slots;
    image_slots.push_back();
    image_slots[0].initialise(dt::hash_string32("IMGUI_SAMPLER"), gfx::SHADER_RESOURCE_IMAGE_SAMPLER, 11, 0, 0, gfx::SHADER_STAGE_ALL);
    m_desc.initialise({ }, image_slots);

    m_pool.initialise(&m_desc, 1);

    ImGui_GfxDevice_ImplVulkan_InitInfo info{ };

    gfx::shader_pass_outputs outputs;
    outputs.color_output_count = 1;
    outputs.color_outputs[0] = gfx::format::R8G8B8A8_SRGB;

    info.Device = reinterpret_cast<gfx::device_vk*>(gfx::driver::get_device());
    info.Pool = &m_pool;
    info.OutputDescription = outputs;
    info.MinImageCount = GFX_RI_FRAMES_IN_FLIGHT;
    info.ImageCount = GFX_RI_FRAMES_IN_FLIGHT;
    info.MSAASamples = gfx::sample_count_flag_bits::count_1;

    ImGui_GfxDevice_ImplVulkan_Init(&info);
    m_frameStart = sys::now();
}

Context::~Context()
{
    ImGui_ImplVulkan_DestroyFontsTexture();
    GFX_CALL(destroy_descriptor_pool, &m_pool);
    GFX_CALL(destroy_descriptor_table_desc, &m_desc);

    ImGui_ImplVulkan_Shutdown();

    ImGui::DestroyContext();
}

void Context::begin_frame()
{
    ImGui_ImplVulkan_NewFrame();

    // ensure frame buffers are the right size
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(f32_cast(m_window->get_extent().x), f32_cast(m_window->get_extent().y));

    // delta time
    sys::moment now = sys::now();
    sys::nanoseconds difference = std::chrono::floor<sys::nanoseconds>(now - m_frameStart);
    io.DeltaTime = difference.count() / 1e9f;
    m_frameStart = now;

    ImGui::NewFrame();
}

void Context::end_frame()
{
    ImGui::Render();
}

void Context::render(gfx::graphics_context& context)
{
    ImGui_GfxDevice_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context);
}

} // mygui