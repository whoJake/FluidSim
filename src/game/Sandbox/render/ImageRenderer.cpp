#include "ImageRenderer.h"
#include "graphics/Shader.h"

ImageRenderer::ImageRenderer(vk::RenderContext& context, Window* window, mygui::Context** imguiContext) :
    m_context(context)
{
    initialize(window, imguiContext);
}

ImageRenderer::~ImageRenderer()
{
    delete m_imguiContext;
}

void ImageRenderer::render_image(vk::Image* image)
{
    vk::CommandBuffer& cmdBuffer = m_context.begin(vk::CommandBuffer::ResetMode::AlwaysAllocate);
    vk::RenderTarget& currTarget = m_context.get_active_frame().get_render_target();

    vk::Framebuffer& currFramebuffer = m_context.get_device().get_resource_cache().request_framebuffer(currTarget, *m_renderPass);

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr, &currFramebuffer, 0);

    const vk::Image& targetImage = currTarget.get_image_views().front().get_image();

    VkImageSubresourceLayers colorSubresource
    {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        0,
        1
    };

    VkExtent3D srcExtent = image->get_extent();
    VkExtent3D dstExtent = targetImage.get_extent();

    VkImageBlit region{ };
    region.srcSubresource = colorSubresource;
    region.dstSubresource = colorSubresource;
    region.srcOffsets[0] = { 0, 0, 0 };
    region.srcOffsets[1] =
    { 
        static_cast<int32_t>(srcExtent.width),
        static_cast<int32_t>(srcExtent.height),
        static_cast<int32_t>(srcExtent.depth)
    };
   
    region.dstOffsets[0] = { 0, 0, 0 };
    region.dstOffsets[1] =
    { 
        static_cast<int32_t>(dstExtent.width),
        static_cast<int32_t>(dstExtent.height),
        static_cast<int32_t>(dstExtent.depth)
    };

    VkFilter filter{ VK_FILTER_NEAREST };

    cmdBuffer.blit_image(*image, VK_IMAGE_LAYOUT_GENERAL, targetImage, VK_IMAGE_LAYOUT_GENERAL, { region }, filter);

    std::vector<VkClearValue> clearColours;

    cmdBuffer.begin_render_pass(&currTarget, *m_renderPass, currFramebuffer, clearColours);

    VkViewport viewport{ };
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = f32_cast(currTarget.get_extent().width);
    viewport.height = f32_cast(currTarget.get_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    cmdBuffer.set_viewport(viewport);

    m_imguiContext->render(&cmdBuffer);

    cmdBuffer.end_render_pass();

    cmdBuffer.end();
    m_context.submit_and_end(cmdBuffer);
}

void ImageRenderer::initialize(Window* window, mygui::Context** myguiContext)
{
    // Create render pass
    std::vector<vk::Attachment> attachments({
        { VK_FORMAT_R8G8B8A8_UNORM,  VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT }
    });

    std::vector<vk::LoadStoreInfo> infos({
        { VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE }
    });

    std::vector<vk::SubpassInfo> subpassInfos{ 
        { 
            { },
            { 0 },
            { },
            true,
            0,
            VK_RESOLVE_MODE_NONE,
            "ImGui Pass"
        },
    };

    m_renderPass = std::make_unique<vk::RenderPass>(vk::RenderPass(m_context.get_device(), attachments, infos, subpassInfos));

    {
        graphics::ShaderDefinition def{ };
        def.vertex = "assets/shaders/basic.vert";
        def.fragment = "assets/shaders/basic.frag";
        def.renderPass = m_renderPass.get();
        def.subpass = 0;

        graphics::Shader shader(m_context, &def);

        int i = 0;
    }

    *myguiContext = new mygui::Context(
        window,
        &m_context,
        m_renderPass.get());

    m_imguiContext = *myguiContext;
}