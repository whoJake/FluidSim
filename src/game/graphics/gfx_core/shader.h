#pragma once
#include "types.h"
#include "gfxdefines.h"

namespace gfx
{

using shader_hash_t = u32;

class shader_resource
{
public:
    shader_resource() = default;
    ~shader_resource() = default;

    DEFAULT_COPY(shader_resource);
    DEFAULT_MOVE(shader_resource);

    void initialise(shader_hash_t hash, shader_resource_type type, u32 slot_index, u32 array_size, u32 resource_size);

    shader_hash_t get_hash() const;
    shader_resource_type get_type() const;
    shader_stage_flags get_stages() const;
    u32 get_slot_index() const;
    u32 get_array_size() const;
    u32 get_resource_size() const;
private:
    shader_hash_t m_hash;
    shader_resource_type m_type;
    u32 m_slotIndex;
    u32 m_arraySize;

    u32 m_resourceSize;
};

class shader_resource_group
{
public:
    shader_resource_group() = default;
    ~shader_resource_group() = default;

    void initialise(u32 group_index, std::vector<shader_resource>&& resources);

    u32 get_index() const;
    const shader_resource& get_slot(u32 slot_index) const;
private:
    bool validate_resources() const;
private:
    u32 m_index;
    std::vector<shader_resource> m_resources;
};

class shader_stage
{
public:
    shader_stage() = default;
    ~shader_stage() = default;

    void initialise(const char* entry_point, std::vector<shader_resource_group>&& groups, shader_stage_flags stage_flags);

    const char* get_entry_point() const;
    shader_stage_flags get_stage_flags() const;

private:
    bool validate_groups() const;
private:
    const char* m_entryPoint;
    std::vector<shader_resource_group> m_groups;
    shader_stage_flags m_stages;
};

class shader
{
public:
    shader() = default;
    ~shader() = default;

    void initialise(shader_hash_t hash, std::vector<shader_stage>&& stages, void* pImpl);

    shader_hash_t get_hash() const;
    const std::vector<shader_stage>& get_stages() const;

    GFX_HAS_IMPL(m_pImpl);
private:
    bool validate_stages() const;
private:
    void* m_pImpl;
    shader_hash_t m_hash;
    std::vector<shader_stage> m_stages;
};

} // gfx