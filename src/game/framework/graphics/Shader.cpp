#include "Shader.h"

#include "system/device.h"

#include "core/Device.h"
#include "core/Pipeline.h"
#include "core/PipelineLayout.h"
#include "core/ShaderModule.h"
#include "rendering/RenderContext.h"
#include "rendering/ContextBackedBuffer.h"

namespace fw
{
namespace gfx
{

Shader::Shader(vk::RenderContext& context,
		       const ShaderDefinition* definition,
	           vk::RenderPass* renderPass,
			   u32 subpass) :
	m_name(definition->name),
	m_context(context)
{
	initialise_layout(definition);
	initialise_pipeline(definition->metadata, renderPass, subpass);

	const std::vector<vk::ShaderResource> resources = m_layout->get_resources(vk::ShaderResourceType::BufferUniform);

	for( const vk::ShaderResource& resource : resources )
	{
		// set 2 is what is reserved for shader uniform buffer usage
		if( resource.set != custom_set_idx )
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
	for( const auto& [_, proxy] : m_resourceOffsets )
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
	for( const auto& [ref, proxy] : m_resourceOffsets )
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

u64 Shader::get_descriptor_set_layout_count() const
{
	return m_layout->get_descriptor_set_count();
}

const vk::DescriptorSetLayout& Shader::get_descriptor_set_layout(u32 idx) const
{
	return m_layout->get_descriptor_set_layout(idx);
}

void Shader::initialise_layout(const ShaderDefinition* definition)
{
	sys::fi_device device;
	std::vector<vk::ShaderModule*> pModules;

	// require atleast a vertex and fragment shader
	if( !definition->vertex
		|| !definition->fragment )
	{
		QUITFMT("A shader is required to have a vertex and fragment shader.");
	}

	device.open(definition->vertex);
	if( !device.is_open() )
	{
		QUITFMT("Shader file {} couldn't be opened.", definition->vertex);
	}

	// vertex
	{
		std::vector<u8> src(device.size());
		device.read(src.data(), src.size());

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
		std::vector<u8> src(device.size());
		device.read(src.data(), src.size());

		vk::ShaderModule& shader = m_context.get_device().get_resource_cache().request_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, src, "main");
		pModules.push_back(&shader);
	}

	device.close();

	m_layout = std::make_unique<vk::PipelineLayout>(m_context.get_device(), pModules);

	std::unordered_map<u32, std::vector<vk::ShaderResource>> resources;
	u32 maxSetIdx = 0;

	for( vk::ShaderModule* module : pModules )
	{
		for( vk::ShaderResource resource : module->get_resources() )
		{
			resources[resource.set].push_back(resource);
			maxSetIdx = std::max(maxSetIdx, resource.set);
		}
	}
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

	vk::RasterizationState rasterizer{ };
	// rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	state.set_rasterization_state(rasterizer);

	VkVertexInputBindingDescription posBindingDesc{ };
	posBindingDesc.binding = 0;
	posBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	posBindingDesc.stride = sizeof(glm::vec4) * 2;

	VkVertexInputAttributeDescription posAttrDesc{ };
	posAttrDesc.binding = 0;
	posAttrDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	posAttrDesc.location = 0;
	posAttrDesc.offset = 0;

	VkVertexInputAttributeDescription nmlAttrDesc{ };
	nmlAttrDesc.binding = 0;
	nmlAttrDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	nmlAttrDesc.location = 1;
	nmlAttrDesc.offset = sizeof(glm::vec4);

	vk::VertexInputStageState inputstate;
	inputstate.bindings.push_back(posBindingDesc);
	inputstate.attributes.push_back(posAttrDesc);
	inputstate.attributes.push_back(nmlAttrDesc);

	state.set_vertex_input_state(inputstate);

	m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_context.get_device(), nullptr, state);
}

const mtl::hash_string& Shader::get_name() const
{
	return m_name;
}

} // gfx
} // fw