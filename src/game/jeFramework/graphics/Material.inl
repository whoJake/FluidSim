#include "Material.h"

namespace graphics
{

template<typename T>
void Material::set_variable_value(mtl::hash_string variable, T value)
{
	if( is_read_only() )
	{
		SYSLOG_WARN("Material is readonly, not changing variable.");
		return;
	}

	 const Shader::ResourceProxy* resource = m_shader->lookup_resource(variable);
	 if( !resource )
	 {
		 SYSLOG_WARN("Resource {} doesn't exist on shader {}.", variable.get_hash(), m_shader->get_name());
		 return;
	 }

	 if( sizeof(T) != resource->size )
	 {
		 SYSLOG_WARN("Invalid size {} of type T ({}). {} expected.", sizeof(T), typeid(T).name(), resource->size);
		 return;
	 }

	 memcpy(m_buffer->map() + resource->offset, &value, sizeof(T));
}

template<typename T>
T Material::get_variable_value(mtl::hash_string variable) const
{
	if( is_read_only() )
	{
		SYSLOG_WARN("Material variable is not accessable.");
		return { };
	}

	const Shader::ResourceProxy* resource = m_shader->lookup_resource(variable);
	if( !resource )
	{
		SYSLOG_WARN("Resource {} doesn't exist on shader {}.", variable.get_hash(), m_shader->get_name());
		return;
	}

	if( sizeof(T) != resource.size )
	{
		SYSLOG_WARN("Invalid size {} of type T ({}). {} expected.", sizeof(T), typeid(T).name(), resource->size);
		return;
	}

	T retval{ };
	memcpy(&retval, m_buffer->map() + resource->offset, sizeof(T));
	return retval;
}

} // graphics