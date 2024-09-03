#include "Material.h"

#include "core/Device.h"
#include "core/DescriptorSet.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderFrame.h"
#include "core/BufferView.h"

namespace fw
{
namespace gfx
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
		m_buffer = std::make_unique<UniformBuffer>(
			context,
			size,
			UniformBufferType::Dynamic
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

	if( Shader::custom_set_idx >= m_shader->get_descriptor_set_layout_count() )
	{
		return;
	}

	const vk::DescriptorSet& set = m_context.get_active_frame().request_descriptor_set(
		m_shader->get_descriptor_set_layout(),
		0,
		get_descriptor_buffer_infos()
	);

	set.write_buffers(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1);
	buffer.bind_descriptor_set(set, Shader::custom_set_idx);
}

const Shader& Material::get_shader() const
{
	return *m_shader;
}

std::vector<VkDescriptorBufferInfo> Material::get_descriptor_buffer_infos() const
{
	vk::BufferView view = m_buffer->get_buffer_view(m_context.get_frame_index());

	std::vector<VkDescriptorBufferInfo> infos;
	infos.emplace_back(view.get_descriptor_info());
	return infos;
}

} // gfx
} // fw