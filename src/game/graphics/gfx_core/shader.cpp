#include "shader.h"

#include "Driver.h"

namespace gfx
{

const shader* shader_manager::find_shader(sys::hash_string name)
{
    auto it = std::lower_bound(
        get().m_loadedShaders.begin(),
        get().m_loadedShaders.end(),
        name,
        [](const std::unique_ptr<shader>& comp, const sys::hash_string& val)
        {
            return comp->get_name().get_hash() < val.get_hash();
        });
    if( it != get().m_loadedShaders.end() )
    {
        shader* found = it->get();

        if( found->get_name().get_hash() == name.get_hash() )
            return it->get();
    }

    return nullptr;
}

void shader_manager::load(const char* path)
{
    sys::hash_string name(path);
    dt::unique_ptr<shader> val = dt::make_unique<shader>();
    val->initialise(name);


}

shader_manager& shader_manager::get()
{
    static shader_manager instance;
    return instance;
}

const descriptor_table& shader::get_table(descriptor_table_type type) const
{
    return m_tables[type];
}

const descriptor_binding* shader::find_binding(descriptor_table_type type, const sys::hash_string& name) const
{
    return m_tables[type].find_binding(name);
}

void shader::initialise(const sys::hash_string& name)
{
    m_name = name;
}

void shader::set_impl(void* pImpl)
{
    m_pImpl = pImpl;
}

void shader::set_table(descriptor_table_type type, descriptor_table&& table)
{
    m_tables[type] = std::move(table);
}

void descriptor_binding::initialise(sys::hash_string name, u64 resource_size, shader_resource_type type, shader_stage_flags flags)
{
    m_name = name;
    m_size = resource_size;
    m_type = type;
    m_visibility = flags;
}

const sys::hash_string& descriptor_binding::get_name() const
{
    return m_name;
}

u64 descriptor_binding::size() const
{
    return m_size;
}

shader_resource_type descriptor_binding::get_resource_type() const
{
    return m_type;
}

shader_stage_flags descriptor_binding::get_visibility() const
{
    return m_visibility;
}

void descriptor_table::initialise(u64 binding_count)
{
    m_bindings = dt::array<descriptor_binding>(binding_count);
}

void descriptor_table::set_binding(u64 index, descriptor_binding&& binding)
{
    GFX_ASSERT(index < m_bindings.size(), "Setting binding {} of descriptor table of size {}.", index, m_bindings.size());
    GFX_ASSERT(find_binding(binding.get_name()) == nullptr, "Binding with the name {} already exists in this descriptor table.", binding.get_name().try_get_str());
    m_bindings[index] = std::move(binding);
}

const descriptor_binding& descriptor_table::get_binding(u64 index) const
{
    return m_bindings[index];
}

const descriptor_binding* descriptor_table::find_binding(const sys::hash_string& name) const
{
    return find_binding(name.get_hash());
}

const descriptor_binding* descriptor_table::find_binding(u64 name_hash) const
{
    // Improve speed, helper sorted mapping?
    for( auto it = m_bindings.begin(); it != m_bindings.end(); ++it )
    {
        if( it->get_name().get_hash() == name_hash )
            return &(*it);
    }

    return nullptr;
}

} // gfx