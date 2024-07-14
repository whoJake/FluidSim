#include "ImGuiContext.h"
#include "imgui.h"
#include "glfw3.h"
#include "backends/imgui_impl_vulkan.h"

#include "core/Device.h"
#include "core/Instance.h"

namespace mygui
{

static void check_vk_result(VkResult result)
{
    if( result != VK_SUCCESS )
        QUITFMT("ImGui vulkan error: ", u32_cast(result));
}

Context::Context(Window* window, vk::RenderContext* renderContext, vk::RenderPass* renderPass) :
    m_window(window),
    m_renderContext(renderContext)
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    viewport->PlatformHandle = window->get_native_handle();
    // viewport->PlatformHandleRaw = glfwGetWin32Window(glfwWindow->get_native_handle());

    // create descriptor pool
    std::array<VkDescriptorPoolSize, 1> poolSizes
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    };
    VkDescriptorPoolCreateInfo pInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    pInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pInfo.maxSets = 1;
    pInfo.poolSizeCount = u32_cast(poolSizes.size());
    pInfo.pPoolSizes = poolSizes.data();
    check_vk_result(vkCreateDescriptorPool(renderContext->get_device().get_handle(), &pInfo, nullptr, &m_pool));

    // vulkan impl init
    ImGui_ImplVulkan_InitInfo info{ };
    info.Instance = renderContext->get_device().get_gpu().get_instance().get_handle();
    info.PhysicalDevice = renderContext->get_device().get_gpu().get_handle();
    info.Device = renderContext->get_device().get_handle();

    const vk::Queue& queue = renderContext->get_device().get_queue_by_flags(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, 0);
    info.QueueFamily = queue.get_family_index();
    info.Queue = queue.get_handle();
    info.DescriptorPool = m_pool;

    //info.PipelineCache = nullptr;
    info.RenderPass = renderPass->get_handle();
    info.Subpass = 0;
    //info.Allocator = nullptr;

    info.MinImageCount = renderContext->get_swapchain_properties().imageCount;
    info.ImageCount = renderContext->get_swapchain_properties().imageCount;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.CheckVkResultFn = &check_vk_result;

    ImGui_ImplVulkan_Init(&info);
    ImGui_ImplVulkan_CreateFontsTexture();

    m_frameStart = sys::now();
}

Context::~Context()
{
    ImGui_ImplVulkan_DestroyFontsTexture();
    vkDestroyDescriptorPool(m_renderContext->get_device().get_handle(), m_pool, nullptr);

    ImGui_ImplVulkan_Shutdown();

    ImGui::DestroyContext();
}

void Context::begin_frame()
{
    ImGui_ImplVulkan_NewFrame();

    // ensure frame buffers are the right size
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(f32_cast(m_window->get_properties().extent.width), f32_cast(m_window->get_properties().extent.height));

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

void Context::render(vk::CommandBuffer* commandBuffer)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->get_handle());
}

} // mygui