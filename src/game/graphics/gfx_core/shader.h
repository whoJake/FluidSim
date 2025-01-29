#pragma once
#include "types.h"
#include "gfxdefines.h"
#include "system/hash_string.h"

namespace gfx
{

using resource_hash_t = u32;

// Descriptor / Binding
class resource_slot
{
public:
    resource_slot() = default;
    ~resource_slot() = default;

    DEFAULT_COPY(resource_slot);
    DEFAULT_MOVE(resource_slot);

    void initialise(resource_hash_t hash, u32 slot_index, shader_resource_type type, u32 array_size, u32 resource_size);

    void add_stage(shader_stage_flag_bits stage);
    void set_stages(shader_stage_flags stages);

    const shader_stage_flags& get_stages() const;
    u32 get_index() const;

    resource_hash_t get_hash() const;
    const shader_resource_type& get_type() const;
    u32 get_array_size() const;
    u32 get_resource_size() const;
private:
    resource_hash_t m_hash;
    shader_stage_flags m_stages;
    u32 m_index;

    shader_resource_type m_type;
    u32 m_arraySize;

    u32 m_resourceSize;
};

// Descriptor Table / Descriptor Set
class resource_group
{
public:
    resource_group() = default;
    ~resource_group() = default;

    void initialise(u32 index);

    void add_slot(resource_slot&& slot);
    const resource_slot& get_slot(u32 index) const;
    u32 get_index() const;

    DEFAULT_COPY(resource_group);
    DEFAULT_MOVE(resource_group);
private:
    std::vector<u32> m_slotMap;
    std::vector<resource_slot> m_slots;
    u32 m_index;
    u32 m_unused;
};

class shader_source
{
public:
    shader_source() = default;
    ~shader_source() = default;

    void initialise(const char* entry_point, std::vector<u32>&& compiled_source);

    const char* const get_entry_point() const;
    const std::vector<u32>& get_source() const;
private:
    const char* m_entryPoint;
    std::vector<u32> m_source;
};

struct shader_stage_sources
{
    shader_source vertex;
    shader_source fragment;
};

using shader_name = sys::hash_string;

class shader
{
public:
    shader() = default;
    ~shader() = default;

    void initialise(shader_name name, std::vector<resource_group>&& groups, shader_stage_sources&& sources);

    const shader_name& get_name() const;
    const resource_group& get_group(u32 index) const;

    GFX_HAS_IMPL(m_pImpl);
private:
    shader_name m_name;
    std::vector<resource_group> m_groups;
    void* m_pImpl;
};

} // gfx