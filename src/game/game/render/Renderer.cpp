#include "Renderer.h"

#include "rendering/RenderContext.h"

#include "core/Buffer.h"
#include "core/RenderPass.h"

#include "scene/spatial/components/Camera.h"

#include "graphics/Shader.h"
#include "graphics/Material.h"

Renderer::Renderer(vk::RenderContext& context) :
    m_context(context),
    m_state(),
    m_streamer(context)
{
    create_renderpass();
    restart();
}

Renderer::~Renderer()
{
    m_context.get_device().wait_idle();

    m_frameData.reset();
    m_cameraData.reset();
    m_modelData.reset();

    for( auto& [name, handle] : m_streaming )
    {
        m_streamer.discard(handle);
    }

    m_meshes.clear();

    m_material.reset();
    m_shader.reset();

    m_renderPass.reset();
}

void Renderer::pre_render(Scene* scene, float deltaTime)
{
    m_streamer.resolve_requests();
    // m_context.get_device().wait_idle();

    RenderFrameData data
    {
        deltaTime,
        m_state.frameIndex,
        m_context.get_frame_index()
    };
    u8* pFrameData = get_frame_data_view().map();
    memcpy(pFrameData, &data, sizeof(RenderFrameData));

    m_state.activeCameras = 0;
    m_state.models = 0;
    m_state.drawList = { };

    for( Entity* entity : scene->get_all_entities() )
    {
        if( entity->has_component<CameraComponent>() )
        {
            add_camera(entity);
        }

        if( entity->has_component<RenderableMeshComponent>() )
        {
            add_renderable(entity);
        }
    }

    m_state.frameIndex++;
}

void Renderer::render()
{
    vk::CommandBuffer& buffer = m_context.begin(vk::CommandBuffer::ResetMode::AlwaysAllocate);

    // start simple, just render directly onto the swapchain images.
    vk::RenderTarget& target = m_context.get_active_frame().get_render_target();
    vk::Framebuffer& framebuffer = m_context.get_device().get_resource_cache().request_framebuffer(target, *m_renderPass);

    buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr, &framebuffer, 0);

    // clear values
    std::vector<VkClearValue> clear;
    VkClearValue clearColor{ };
    clearColor.color = { .1f, .1f, .1f, 1.f };
    VkClearValue clearDepth{ };
    clearDepth.depthStencil = { 1.f, 0 };

    clear.push_back(clearColor);
    clear.push_back(clearDepth);

    buffer.begin_render_pass(&target, *m_renderPass, framebuffer, clear);

    // Remember to setup viewport and scissor. We render to the whole screen with this so its fine to do full
    VkViewport viewport{ };
    viewport.x = 0.f;
    viewport.y = f32_cast(target.get_extent().height);
    viewport.width = f32_cast(target.get_extent().width);
    viewport.height = -f32_cast(target.get_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{ };
    scissor.offset = { 0, 0 };
    scissor.extent = target.get_extent();

    buffer.set_viewport(viewport);
    buffer.set_scissor(scissor);

    // always bind same material
    m_material->bind(buffer);

    // SYSLOG_INFO("Frame Data");
    const vk::DescriptorSet& frameDataSet = m_context.get_active_frame().request_descriptor_set(
        m_material->get_shader().get_descriptor_set_layout(0),
        0,
        { get_frame_data_view().get_descriptor_info() }
    );

    frameDataSet.write_buffers(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1);
    buffer.bind_descriptor_set(frameDataSet, 0);

    for( u32 camIdx = 0; camIdx < m_state.activeCameras; camIdx++ )
    {
        // SYSLOG_INFO("Camera Data {}", camIdx);
        const vk::DescriptorSet& cameraDataSet = m_context.get_active_frame().request_descriptor_set(
            m_material->get_shader().get_descriptor_set_layout(1),
            0,
            { get_camera_data_view(camIdx).get_descriptor_info() }
        );

        cameraDataSet.write_buffers(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1);
        buffer.bind_descriptor_set(cameraDataSet, 1);

        for( u32 modelIdx = 0; modelIdx < m_state.models; modelIdx++ )
        {
            fw::gfx::DrawData* drawable = m_state.drawList[modelIdx];
            const std::vector<fw::gfx::DrawMesh>& meshes = drawable->get_meshes();

            // SYSLOG_INFO("Model Data {}", modelIdx);
            const vk::DescriptorSet& modelDataSet = m_context.get_active_frame().request_descriptor_set(
                m_material->get_shader().get_descriptor_set_layout(2),
                0,
                { get_model_data_view(modelIdx).get_descriptor_info() }
            );

            modelDataSet.write_buffers(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1);
            buffer.bind_descriptor_set(modelDataSet, 2);

            u32 vertexCount = meshes.front().vertexCount;
            vk::BufferView vertex(drawable->get_data(), 0, vertexCount * sizeof(glm::vec4) * 2);

            buffer.bind_vertex_buffers(vertex, 0);
            buffer.draw(vertexCount);
        }
    }

    buffer.end_render_pass();
    buffer.end();
    m_context.submit_and_end(buffer);
}

void Renderer::restart()
{
    m_context.get_device().wait_idle();

    // read settings?
    u32 imageCount = m_context.get_swapchain_properties().imageCount;
    m_frameData = std::make_unique<fw::gfx::UniformBuffer>(
        m_context,
        sizeof(RenderFrameData),
        fw::gfx::UniformBufferType::Dynamic
    );

    m_cameraData = std::make_unique<fw::gfx::UniformBuffer>(
        m_context,
        sizeof(RenderCameraData),
        fw::gfx::UniformBufferType::Dynamic,
        max_cameras
    );

    m_modelData = std::make_unique<fw::gfx::UniformBuffer>(
        m_context,
        sizeof(RenderModelData),
        fw::gfx::UniformBufferType::Dynamic,
        max_models
    );

    m_frameData->map();
    m_cameraData->map();
    m_modelData->map();

    fw::gfx::ShaderDefinition shader
    {
        mtl::hash_string("shader1"),
        "assets/shaders/modules/vertex/basic_unknown.vert",
        "assets/shaders/modules/fragment/basic_unknown.frag",
        ""
    };

    m_shader = std::make_unique<fw::gfx::Shader>(
        m_context,
        &shader,
        m_renderPass.get(),
        0
    );

    m_material = std::make_unique<fw::gfx::Material>(
        m_context,
        m_shader.get(),
        0
    );
}

void Renderer::add_camera(Entity* entity)
{
    u32 cameraIdx = m_state.activeCameras++;
    CameraComponent* camera = entity->get_component<CameraComponent>();

    RenderCameraData data
    {
        camera->calculate_matrix(),
        entity->transform().get_matrix_as_view(),
        
        glm::mat4(1.f),

        entity->transform().get_position(),
        glm::vec2(camera->get_near_z(), camera->get_far_z())
    };

    data.projection_view_matrix = data.projection_matrix * data.view_matrix;

    u8* pGpuData = get_camera_data_view(cameraIdx).map();
    memcpy(pGpuData, &data, sizeof(RenderCameraData));
}

void Renderer::add_renderable(Entity* entity)
{
    RenderableMeshComponent* mesh = entity->get_component<RenderableMeshComponent>();

    auto itMesh = m_meshes.find(mesh->get_hash_string());

    if( itMesh == m_meshes.end() )
    {
        load_draw_data(mesh);
        return;
    }

    fw::gfx::DrawData* drawData = itMesh->second.get();
    if( drawData->get_state() == fw::gfx::DrawDataState::LOADING )
    {
        // check the handle
        auto itHandle = m_streaming.find(mesh->get_hash_string());
        fw::gfx::StreamHandle* handle = itHandle->second;
        if( m_streamer.is_loaded(handle) )
        {
            std::unique_ptr<vk::Buffer> data = m_streamer.access(handle);
            m_streaming.erase(itHandle);
            drawData->set_data(std::move(data));
            drawData->set_state(fw::gfx::DrawDataState::LOADED);
        }
    }

    if( drawData->get_state() != fw::gfx::DrawDataState::LOADED )
    {
        return;
    }

    // Add to render list.
    u32 modelIdx = m_state.models++;
    RenderModelData data
    {
        entity->transform().get_matrix()
    };

    vk::BufferView modelBuffer = get_model_data_view(modelIdx);
    u8* pGpuData = modelBuffer.map();
    memcpy(pGpuData, &data, sizeof(RenderModelData));

    m_state.drawList.push_back(drawData);
}

vk::BufferView Renderer::get_frame_data_view() const
{
    return m_frameData->get_buffer_view(m_context.get_frame_index());
}

vk::BufferView Renderer::get_camera_data_view(u32 idx) const
{
    return m_cameraData->get_buffer_view(m_context.get_frame_index(), idx);
}

vk::BufferView Renderer::get_model_data_view(u32 idx) const
{
    return m_modelData->get_buffer_view(m_context.get_frame_index(), idx);
}

void Renderer::create_renderpass()
{
    // r8g8b8a8 color
    // 32 bit depth
    std::vector<vk::Attachment> attachments({
        { VK_FORMAT_R8G8B8A8_SRGB,  VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT },
        { VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT },
    });

    // clear color
    // clear depth
    std::vector<vk::LoadStoreInfo> infos({
        { VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE },
        { VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE },
    });

    std::vector<vk::SubpassInfo> subpassInfos{ 
        { 
            { },
            { 0 },
            { },
            false,
            0,
            VK_RESOLVE_MODE_NONE,
            "Main Subpass"
        },
    };

    m_renderPass = std::make_unique<vk::RenderPass>(
        m_context.get_device(),
        attachments,
        infos,
        subpassInfos
    );
}

void Renderer::load_draw_data(RenderableMeshComponent* mesh)
{
    mesh->load();

    const mtl::submesh& submesh = mesh->get_mesh().get_submesh(0);
    std::vector<glm::vec4> data;

    const auto& vertices = submesh.get_channel(0);
    const auto& normals = submesh.get_channel(1);

    for( u64 i = 0; i < vertices.size(); i++ )
    {
        data.push_back(vertices[i]);
        data.push_back(normals[i]);
    }

    std::vector<fw::gfx::DrawMesh> meshes;
    meshes.push_back(fw::gfx::DrawMesh{ 0, u32_cast(submesh.get_channel(0).size()), 0 });
    std::vector<mtl::hash_string> matDeps;
    matDeps.push_back(mtl::hash_string("material1"));
    mtl::hash_string name = mesh->get_hash_string();

    mesh->unload();

    m_meshes.emplace(
        std::piecewise_construct,
        std::tuple(name),
        std::tuple(std::make_unique<fw::gfx::DrawData>(meshes, matDeps))
    );

    fw::gfx::DrawData* drawData = m_meshes[name].get();

    std::unique_ptr<vk::Buffer> cpuBuffer = std::make_unique<vk::Buffer>(
        m_context.get_device(),
        data.size() * sizeof(glm::vec4),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST
    );

    u8* pCpuData = cpuBuffer->map();
    memcpy(pCpuData, data.data(), data.size() * sizeof(glm::vec4));
    cpuBuffer->unmap();

    fw::gfx::StreamHandle* handle = m_streamer.request(std::move(cpuBuffer), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    drawData->set_state(fw::gfx::DrawDataState::LOADING);
    m_streaming.emplace(
        std::piecewise_construct,
        std::tuple(name),
        std::tuple(handle)
    );
}