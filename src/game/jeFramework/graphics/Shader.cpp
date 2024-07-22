#include "Shader.h"

#include "device/fiDevice.h"

#include "core/Device.h"
#include "core/Pipeline.h"
#include "core/PipelineLayout.h"
#include "core/ShaderModule.h"
#include "rendering/RenderContext.h"
#include "rendering/ContextBackedBuffer.h"

namespace graphics
{

Shader::Shader(vk::RenderContext& context, ShaderDefinition* definition) :
	m_context(context)
{
	initialise_layout(definition);
	initialise_pipeline(definition->metadata, definition->renderPass, definition->subpass);

	const std::vector<vk::ShaderResource> resources = m_layout->get_resources(vk::ShaderResourceType::BufferUniform);
	
	for( const vk::ShaderResource& resource : resources )
	{
		// set 2 is what is reserved for shader uniform buffer usage
		if( resource.set != 2 )
			break;

		for( vk::ShaderStructMember member : resource.structMembers )
		{
			m_resourceOffsets.emplace(
				std::piecewise_construct,
				std::tuple(resource.name + "." + member.name),
				std::tuple(ResourceProxy{ resource.binding, member.offset, member.size }));
		}
	}
}

Shader::~Shader()
{ }

void Shader::initialise_layout(ShaderDefinition* definition)
{
	fiDevice device;
	std::vector<vk::ShaderModule*> pModules;

	// require atleast a vertex and fragment shader
	if( !definition->vertex
		|| !definition->fragment )
	{
		QUITFMT("A shader is required to have a vertex and fragment shader.");
	}

	// required to have a renderpass
	if( !definition->renderPass )
	{
		QUITFMT("A renderpass must be passed when creating a shader.");
	}

	device.open(definition->vertex);
	if( !device.is_open() )
	{
		QUITFMT("Shader file {} couldn't be opened.", definition->vertex);
	}

	// vertex
	{
		std::vector<u8> src = device.read(device.get_size());
		vk::ShaderModule& shader = m_context.get_device().get_resource_cache().request_shader_module(VK_SHADER_STAGE_VERTEX_BIT, src, "main");
		pModules.push_back(&shader);
	}

	device.open(definition->fragment);
	if( !device.is_open() )
	{
		QUITFMT("Shader file {} couldn't be opened.", definition->fragment);
	}

	// fragment
	{
		std::vector<u8> src = device.read(device.get_size());
		vk::ShaderModule& shader = m_context.get_device().get_resource_cache().request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, src, "main");
		pModules.push_back(&shader);
	}

	device.close();

	m_layout = std::make_unique<vk::PipelineLayout>(m_context.get_device(), pModules);
}

void Shader::initialise_pipeline(const char* metadataFile, vk::RenderPass* renderpass, u32 subpass)
{
	if( !metadataFile )
		GRAPHICS_WARN("No metadata file provided. Using defaults.");

	vk::PipelineState state{ };
	state.set_pipeline_layout(*m_layout);
	state.set_render_pass(*renderpass);
	state.set_subpass_index(subpass);

	m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_context.get_device(), nullptr, state);
}

} // graphics