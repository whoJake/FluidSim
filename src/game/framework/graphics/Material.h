#pragma once

#include "Shader.h"
#include "UniformBuffer.h"
#include "core/CommandBuffer.h"

namespace fw
{
namespace gfx
{

enum MaterialFlagBits
{
	none = 0,
	readonly = (1 << 0),
};

using MaterialFlags = std::underlying_type_t<MaterialFlagBits>;

struct MaterialDefinition
{
	mtl::hash_string name;
	mtl::hash_string shader;

	MaterialFlags flags;
};

class Material
{
public:
	// todo pass initial state of material?
	Material(vk::RenderContext& context, Shader* shader, MaterialFlags flags);
	~Material();

	template<typename T>
	void set_variable_value(mtl::hash_string variable, T value);

	template<typename T>
	T get_variable_value(mtl::hash_string variable) const;

	bool is_read_only() const;

	void bind(vk::CommandBuffer& buffer) const;

	const Shader& get_shader() const;

	std::vector<VkDescriptorBufferInfo> get_descriptor_buffer_infos() const;
private:
	vk::RenderContext& m_context;
	Shader* m_shader;

	std::unique_ptr<UniformBuffer> m_buffer;

	MaterialFlags m_flags;
};

} // gfx
} // fw

#ifndef INC_MATERIAL_INL
#define INC_MATERIAL_INL
#include "Material.inl"
#endif