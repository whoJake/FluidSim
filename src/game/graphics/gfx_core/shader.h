#pragma once
#include "types.h"
#include "gfxdefines.h"
#include "system/hash_string.h"
#include "dt/array.h"

namespace gfx
{

using shader_allocator = dt::zoned_allocator<sys::MEMZONE_SHADERS>;

class descriptor_binding
{
public:
    descriptor_binding() = default;
    ~descriptor_binding() = default;

    void initialise(sys::hash_string name, u64 resource_size, shader_resource_type type, shader_stage_flags visibility);

    u64 get_hash() const;
    u64 size() const;
    shader_resource_type get_resource_type() const;
    shader_stage_flags get_visibility() const;
private:
    sys::hash_string m_name;
    u64 m_size;
    shader_resource_type m_type;
    shader_stage_flags m_visibility;
};

class descriptor_table
{
public:
    descriptor_table() = default;
    ~descriptor_table() = default;

    const descriptor_binding* get_binding(u64 index) const;
    const descriptor_binding* find_binding(const sys::hash_string& name) const;
private:
    dt::array<descriptor_binding> m_bindings;
};

class shader
{
public:
    shader() = default;
    ~shader() = default;

    const descriptor_table& get_table(descriptor_table_type type) const;
    const descriptor_binding& find_binding(descriptor_table_type type, const sys::hash_string& name) const;

    GFX_HAS_IMPL(m_pImpl);
private:
    sys::hash_string m_name;
    dt::inline_array<descriptor_table, DESCRIPTOR_TABLE_COUNT> m_tables;

    void* m_pImpl;
};

} // gfx