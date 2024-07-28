#include "Material.h"

#include "core/Device.h"
#include "core/DescriptorSet.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderFrame.h"

namespace graphics
{

Material::Material(vk::RenderContext& context, Shader* shader, MaterialFlags flags) :
	m_context(context),
	m_shader(shader),
	m_flags(flags)
{
	u64 size = 0;
	for( u32 i = 0; i <= shader->get_binding_count(); i++ )
	{
		size += shader->get_binding_size(i);
	}

	if( size != 0 )
	{
		m_buffer = std::make_unique<vk::Buffer>(
			context.get_device(),
			size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_MEMORY_USAGE_AUTO_PREFER_HOST
		);
	}
}

Material::~Material()
{ }

bool Material::is_read_only() const
{
	return m_flags & MaterialFlagBits::readonly;
}

void Material::bind(vk::CommandBuffer& buffer) const
{
	buffer.bind_pipeline(m_shader->get_pipeline());
	buffer.bind_pipeline_layout(m_shader->get_layout());

	if( !m_buffer )
	{
		return;
	}

	const vk::DescriptorSet& set = m_context.get_active_frame().request_descriptor_set(
		m_shader->get_descriptor_set_layout(),
		0,
		get_descriptor_buffer_infos()
	);

	set.write_buffers(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1);
	buffer.bind_descriptor_set(set);
}

const Shader& Material::get_shader() const
{
	return *m_shader;
}

std::vector<VkDescriptorBufferInfo> Material::get_descriptor_buffer_infos() const
{
	std::vector<VkDescriptorBufferInfo> infos;

	infos.emplace_back(VkDescriptorBufferInfo{
		m_buffer->get_handle(),
		0,
		m_buffer->get_size()
		});

	return infos;
}

} // graphics