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
	m_name(definition->name),
	m_context(context)
{
	initialise_layout(definition);
	initialise_pipeline(definition->metadata, definition->renderPass, definition->subpass);

	const std::vector<vk::ShaderResource> resources = m_layout->get_resources(vk::ShaderResourceType::BufferUniform);
	
	for( const vk::ShaderResource& resource : resources )
	{
		// set 2 is what is reserved for shader uniform buffer usage
		if( resource.set != 0 )
			continue;

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

u32 Shader::get_binding_count() const
{
	u32 maxBinding = u32_min;
	for( const auto&[_, proxy] : m_resourceOffsets )
	{
		maxBinding = std::max(maxBinding, proxy.binding);
	}

	return maxBinding;
}

u64 Shader::get_binding_size(u32 idx) const
{
	const std::vector<vk::ShaderResource> resources = m_layout->get_resources(vk::ShaderResourceType::BufferUniform);
	for( const vk::ShaderResource& resource : resources )
	{
		if( resource.binding == idx )
		{
			return resource.stride;
		}
	}

	return 0;
}

const Shader::ResourceProxy* Shader::lookup_resource(mtl::hash_string location) const
{
	for( const auto&[ref, proxy] : m_resourceOffsets )
	{
		if( ref == location )
		{
			return &proxy;
		}
	}

	return nullptr;
}

vk::PipelineLayout& Shader::get_layout() const
{
	return *m_layout;
}

vk::Pipeline& Shader::get_pipeline() const
{
	return *m_pipeline;
}

std::string_view Shader::get_name() const
{
	return m_name;
}

const vk::DescriptorSetLayout& Shader::get_descriptor_set_layout() const
{
	return *m_descriptorSetLayout;
}

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

	std::vector<vk::ShaderResource> allResources;
	for( vk::ShaderModule* module : pModules )
	{
		for( vk::ShaderResource resource : module->get_resources() )
		{
			if( resource.set == 0 )
			{
				allResources.push_back(resource);
			}
		}
	}

	m_descriptorSetLayout = &m_context.get_device().get_resource_cache().request_descriptor_set_layout(2, pModules, allResources);
}

void Shader::initialise_pipeline(const char* metadataFile, vk::RenderPass* renderpass, u32 subpass)
{
	if( !metadataFile )
		SYSLOG_WARN("No metadata file provided. Using defaults.");

	vk::PipelineState state{ };
	state.set_pipeline_layout(*m_layout);
	state.set_render_pass(*renderpass);
	state.set_subpass_index(subpass);

	vk::ColorBlendState colorstate;
	colorstate.attachments.push_back(vk::ColorBlendAttachmentState());
	state.set_color_blend_state(colorstate);

	m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_context.get_device(), nullptr, state);
}

} // graphics