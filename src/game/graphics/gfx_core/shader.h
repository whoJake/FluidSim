#pragma once
#include "types.h"
#include "gfxdefines.h"
#include "system/hash_string.h"
#include "dt/array.h"
#include "dt/vector.h"
#include "dt/unique_ptr.h"

namespace gfx
{

class Driver;
class shader;

class shader_manager
{
public:
    static const shader* find_shader(sys::hash_string name);
    static void load(const char* path);
private:
    static shader_manager& get();
    dt::vector<dt::unique_ptr<shader, dt::zoned_allocator<MEMZONE_SHADERS>>> m_loadedShaders;
};

class descriptor_binding
{
public:
    descriptor_binding() = default;
    ~descriptor_binding() = default;

    void initialise(sys::hash_string name, u64 resource_size, shader_resource_type type, shader_stage_flags visibility);

    const sys::hash_string& get_name() const;
    u64 size() const;
    shader_resource_type get_resource_type() const;
    shader_stage_flags get_visibility() const;
private:
    sys::hash_string m_name;
    u64 m_size;
    shader_resource_type m_type{ SHADER_RESOURCE_EMPTY };
    shader_stage_flags m_visibility;
};

class descriptor_table
{
public:
    descriptor_table() = default;
    ~descriptor_table() = default;

    void initialise(u64 binding_count);
    void set_binding(u64 index, descriptor_binding&& binding);

    const descriptor_binding& get_binding(u64 index) const;
    const descriptor_binding* find_binding(const sys::hash_string& name) const;
    const descriptor_binding* find_binding(u64 name_hash) const;
private:
    dt::array<descriptor_binding> m_bindings;
};

class pass
{
public:
    pass() = default;
    ~pass() = default;
private:
};

class shader
{
public:
    shader() = default;
    ~shader() = default;
    void initialise(const sys::hash_string& name);
    void set_table(descriptor_table_type type, descriptor_table&& table);
    void set_impl(void* pImpl);

    const sys::hash_string& get_name() const;

    const descriptor_table& get_table(descriptor_table_type type) const;
    const descriptor_binding* find_binding(descriptor_table_type type, const sys::hash_string& name) const;

    GFX_HAS_IMPL(m_pImpl);
private:
    sys::hash_string m_name;
    dt::inline_array<descriptor_table, DESCRIPTOR_TABLE_COUNT> m_tables;

    // VkGraphicsPipeline
    void* m_pImpl;
};

} // gfx