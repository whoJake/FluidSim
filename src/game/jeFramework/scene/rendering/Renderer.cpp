#include "Renderer.h"

#include "core/ShaderModule.h"
#include "core/RenderPass.h"
#include "core/PipelineLayout.h"
#include "core/Pipeline.h"

#include "device/fiDevice.h"

Renderer::Renderer(vk::RenderContext& context) :
    m_context(context)
{
    build_debug_material();
}

Renderer::~Renderer()
{ }

void Renderer::dispatch_render(SceneProxies scene, const std::vector<Camera*>& cameras, uint32_t* fence)
{
    // Ensure fence is not 0
    *fence = 100;

    vk::CommandBuffer& mainCmdBuffer = m_context.begin(vk::CommandBuffer::ResetMode::AlwaysAllocate);
    vk::RenderTarget& activeTarget = m_context.get_active_frame().get_render_target();

    // debug
    vk::Framebuffer& activeFramebuffer = m_context.get_device().get_resource_cache().request_framebuffer(activeTarget, *m_debugMaterial.renderPass);

    mainCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr, &activeFramebuffer, 0);

    VkClearValue color{ };
    color.color = { .2f, .2f, .2f, 1.f };
    VkClearValue depth{ };
    depth.depthStencil = { 1.f, 0 };

    std::vector<VkClearValue> clearColours;
    clearColours.push_back(color);
    clearColours.push_back(depth);

    mainCmdBuffer.begin_render_pass(&activeTarget, *m_debugMaterial.renderPass, activeFramebuffer, clearColours);
    mainCmdBuffer.bind_pipeline(*m_debugMaterial.pipeline);

    VkViewport viewport{ };
    viewport.y = static_cast<float>(activeTarget.get_extent().height);
    viewport.width = static_cast<float>(activeTarget.get_extent().width);
    viewport.height = -static_cast<float>(activeTarget.get_extent().height);
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    mainCmdBuffer.set_viewport(viewport);

    VkRect2D scissor{ };
    scissor.offset = { 0, 0 };
    scissor.extent = activeTarget.get_extent();
    mainCmdBuffer.set_scissor(scissor);

    CameraMatrixData cameraMatrix
    {
        cameras.at(0)->as_projection_matrix(),
        cameras.at(0)->as_view_matrix()
    };

    // Set camera
    mainCmdBuffer.push_constants(
        *m_debugMaterial.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(CameraMatrixData),
        &cameraMatrix);


    for( const auto& entitymap : scene.entities )
    {
        const EntityProxy& entity = entitymap.second;

        auto blueprintIt = scene.blueprints.find(entity.get_bpid());
        if( blueprintIt == scene.blueprints.end() )
        {
            // blueprint not loaded.
            continue;
        }

        const BlueprintProxy& blueprint = blueprintIt->second;

        glm::mat4 modelMatrix = entity.get_model_matrix();

        // Set model
        mainCmdBuffer.push_constants(
            *m_debugMaterial.pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            sizeof(CameraMatrixData),
            sizeof(glm::mat4),
            &modelMatrix);

        std::vector<vk::Buffer*> vertexBuffers = blueprint.get_mesh_proxy().get_vertex_buffers(m_context.get_active_render_frame_index());

        mainCmdBuffer.bind_vertex_buffers(vertexBuffers, 0);
        mainCmdBuffer.bind_index_buffer(*blueprint.get_mesh_proxy().get_index_buffer(m_context.get_active_render_frame_index()), VK_INDEX_TYPE_UINT16);

        mainCmdBuffer.draw_indexed(vk::to_u32(blueprint.get_mesh_proxy().get_index_count()));
    }

    mainCmdBuffer.end_render_pass();
    mainCmdBuffer.end();

    m_context.submit_and_end(mainCmdBuffer); // active frame is set to false here <--

    *fence = 0;
}

void Renderer::build_debug_material()
{
    std::vector<vk::Attachment> attachments({
        { VK_FORMAT_R8G8B8A8_UNORM,  VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT },
        { VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT }});

    std::vector<vk::LoadStoreInfo> infos({
        { VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE },
        { VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE }});

    std::vector<vk::SubpassInfo> subpassInfos({ { 
        { },
        { 0 },
        { },
        false,
        1,
        VK_RESOLVE_MODE_NONE,
        "Subpass" } });

    m_debugMaterial.renderPass = std::make_unique<vk::RenderPass>(vk::RenderPass(m_context.get_device(), attachments, infos, subpassInfos));

    fiDevice device;
    std::string vertPath = "shaders/basic.vert";

    device.open(vertPath.c_str());
    std::vector<uint8_t> vertSrc = device.read(device.get_size());
    device.close();

    std::string fragPath = "shaders/basic.frag";
    device.open(fragPath.c_str());
    std::vector<uint8_t> fragSrc = device.read(device.get_size());
    device.close();

    vk::ShaderModule& vertModule = m_context.get_device().get_resource_cache().request_shader_module(VK_SHADER_STAGE_VERTEX_BIT, vertSrc, "main");
    vk::ShaderModule& fragModule = m_context.get_device().get_resource_cache().request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, fragSrc, "main");

    std::vector<vk::ShaderModule*> modules({ &vertModule, &fragModule });

    m_debugMaterial.pipelineLayout = std::make_unique<vk::PipelineLayout>(vk::PipelineLayout(m_context.get_device(), modules));
    m_debugMaterial.pipelineState.set_pipeline_layout(*m_debugMaterial.pipelineLayout);
    m_debugMaterial.pipelineState.set_render_pass(*m_debugMaterial.renderPass);

    VkVertexInputBindingDescription bindingDescription{ };
    bindingDescription.binding = 0;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDescription.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription attributeDescription{ };
    attributeDescription.binding = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.location = 0;
    attributeDescription.offset = offsetof(Vertex, Vertex::position);

    VkVertexInputAttributeDescription attributeDescription2{ };
    attributeDescription2.binding = 0;
    attributeDescription2.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription2.location = 1;
    attributeDescription2.offset = offsetof(Vertex, Vertex::normal);

    VkVertexInputAttributeDescription attributeDescription3{ };
    attributeDescription3.binding = 0;
    attributeDescription3.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription3.location = 2;
    attributeDescription3.offset = offsetof(Vertex, Vertex::colour);

    vk::VertexInputStageState inputStage{ };
    inputStage.bindings.push_back(bindingDescription);
    inputStage.attributes.push_back(attributeDescription);
    inputStage.attributes.push_back(attributeDescription2);
    inputStage.attributes.push_back(attributeDescription3);

    vk::RasterizationState rast{ };
    rast.polygonMode = Param_wireframe.get() ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rast.cullMode = Param_disable_backface_culling.get() ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

    m_debugMaterial.pipelineState.set_vertex_input_state(inputStage);
    m_debugMaterial.pipelineState.set_rasterization_state(rast);

    vk::ColorBlendState colorstate;
    colorstate.attachments.push_back(vk::ColorBlendAttachmentState());
    m_debugMaterial.pipelineState.set_color_blend_state(colorstate);

    m_debugMaterial.pipeline = std::make_unique<vk::Pipeline>(vk::GraphicsPipeline(m_context.get_device(), VK_NULL_HANDLE, m_debugMaterial.pipelineState));
}