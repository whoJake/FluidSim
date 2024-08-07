#include "SceneRenderer.h"

#include "core/Buffer.h"
#include "core/RenderPass.h"
#include "rendering/RenderContext.h"

#include "graphics/Shader.h"
#include "graphics/Material.h"

#include "scene/Scene.h"
#include "scene/Blueprint.h"
#include "scene/Entity.h"
#include "scene/BlueprintManager.h"

namespace fw
{

SceneRenderer::SceneRenderer(vk::RenderContext& context) :
	m_context(context),
	m_state()
{
	setup_renderpass();
}

void SceneRenderer::pre_render(glm::vec3 position)
{
	// find our visible entities
	m_state.visibleEntities.clear();

	// pre-allocate the size
	u64 entities = 0;
	for( const Scene* scene : m_scenes )
	{
		entities += scene->get_all_entities().size();
	}

	m_state.visibleEntities.reserve(entities);
	for( const Scene* scene : m_scenes )
	{
		for( Entity* entity : scene->get_all_entities() )
		{
			m_state.visibleEntities.emplace_back(entity);
		}
	}

	// ensure that all the our blueprint draw data is loaded/being loaded.
	for( const Entity* entity : m_state.visibleEntities )
	{
		Blueprint* blueprint = BlueprintManager::find_blueprint(entity->get_blueprint());

		// cache our materials
		gfx::MaterialDefinition& material = blueprint->get_material();
		if( m_materialStore.find(material.name) == m_materialStore.end() )
		{
			register_material(material);
		}

		if( m_loadedBlueprints.find(blueprint->get_name()) == m_loadedBlueprints.end() )
		{
			load_blueprint(blueprint);
		}
	}

	// setup the global uniform buffer.
	// I think this would make sense to have a "SceneView" which
	// has a ptr to a Scene + a camera component (target?)
	{
		GlobalSetData data{ };
		VkExtent2D extent{ 1600, 1200 };
		data.projection = glm::perspectiveFov(glm::radians(90.f), f32_cast(extent.width), f32_cast(extent.height), 0.01f, 10'000.f);
		data.view = glm::translate(glm::mat4(1.f), -position);
		data.model = glm::mat4(1.f);
		data.mvp = data.model * data.view * data.projection;

		set_global_buffer(&data);
	}
}

void SceneRenderer::render()
{
	vk::CommandBuffer& buffer = m_context.begin(vk::CommandBuffer::ResetMode::AlwaysAllocate);

	// start simple, just render directly onto the swapchain images.
	vk::RenderTarget& target = m_context.get_active_frame().get_render_target();
	vk::Framebuffer& framebuffer = m_context.get_device().get_resource_cache().request_framebuffer(target, *m_state.renderPass);

	buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr, &framebuffer, 0);

	// clear values
	std::vector<VkClearValue> clear;
	VkClearValue clearColor{ };
	clearColor.color = { .1f, .1f, .1f, 1.f };
	VkClearValue clearDepth{ };
	clearDepth.depthStencil = { 1.f, 0 };

	clear.push_back(clearColor);
	clear.push_back(clearDepth);

	buffer.begin_render_pass(&target, *m_state.renderPass, framebuffer, clear);

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

	for( Entity* entity : m_state.visibleEntities )
	{
		Blueprint* blueprint = BlueprintManager::find_blueprint(entity->get_blueprint());
		if( !blueprint )
			continue;

		// get blueprint draw data

		// bind material
		const gfx::Material& material = *m_materialStore[blueprint->get_material().name];
		material.bind(buffer);

		VkDescriptorBufferInfo globalBufferInfo
		{
			m_state.globalBuffer->get_handle(),
			0,
			sizeof(GlobalSetData)
		};
		const vk::DescriptorSet& globalSet = m_context.get_active_frame().request_descriptor_set(
			material.get_shader().get_descriptor_set_layout(0),
			0,
			{ globalBufferInfo }
		);

		globalSet.write_buffers(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1);
		buffer.bind_descriptor_set(globalSet);

		buffer.bind_vertex_buffers(*m_loadedBlueprints[blueprint->get_name()].vertexBuffer, 0);
		buffer.draw(u32_cast(blueprint->get_mesh().get_submesh(0).get_channel(0).size()));

		buffer.end_render_pass();
		buffer.end();
		m_context.submit_and_end(buffer);
	}
}

void SceneRenderer::add_scene(Scene* scene)
{
	m_scenes.push_back(scene);
}

void SceneRenderer::setup_renderpass()
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

	m_state.renderPass = std::make_unique<vk::RenderPass>(vk::RenderPass(m_context.get_device(), attachments, infos, subpassInfos));

	// create our global buffer
	m_state.globalBuffer = std::make_unique<vk::Buffer>(
		m_context.get_device(),
		sizeof(GlobalSetData),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST
	);
}

void SceneRenderer::initialise_shaders(const std::vector<gfx::ShaderDefinition>& shaders)
{
	for( const gfx::ShaderDefinition& definition : shaders )
	{
		m_shaderStore[definition.name] = std::make_unique<gfx::Shader>(m_context, &definition, m_state.renderPass.get(), 0);
	}
}

void SceneRenderer::register_material(const gfx::MaterialDefinition& definition)
{
	auto itShader = m_shaderStore.find(definition.shader);
	if( itShader == m_shaderStore.end() )
	{
		// cry
		return;
	}

	gfx::Shader* shader = m_shaderStore[definition.shader].get();
	m_materialStore[definition.name] = std::make_unique<gfx::Material>(m_context, shader, definition.flags);
}

void SceneRenderer::load_blueprint(Blueprint* blueprint)
{
	BlueprintDrawData blueprintDrawData;

	mtl::mesh& mesh = blueprint->get_mesh();
	mtl::submesh& submesh = mesh.get_submesh(0);

	u64 vBufferSize = submesh.get_channel(0).size() * sizeof(glm::vec4);
	u64 nBufferSize = submesh.get_channel(1).size() * sizeof(glm::vec4);

	blueprintDrawData.vertexBuffer = std::make_unique<vk::Buffer>(
		m_context.get_device(),
		vBufferSize + nBufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_AUTO_PREFER_HOST
	);

	u8* data = blueprintDrawData.vertexBuffer->map();

	// interleave
	for( u64 i = 0; i < submesh.get_channel(0).size(); i++ )
	{
		u64 offset = i * (sizeof(glm::vec4) * 2);
		glm::vec4 vertex = submesh.get_channel(0)[i];
		glm::vec4 normal = submesh.get_channel(1)[i];

		memcpy(data + offset, &vertex, sizeof(glm::vec4));
		memcpy(data + offset + sizeof(glm::vec4), &normal, sizeof(glm::vec4));
	}

	blueprintDrawData.vertexBuffer->unmap();

	m_loadedBlueprints[blueprint->get_name()] = std::move(blueprintDrawData);
}

void SceneRenderer::set_global_buffer(GlobalSetData* data)
{
	u8* pData = m_state.globalBuffer->map();
	memcpy(pData, data, sizeof(GlobalSetData));
}

} // fw