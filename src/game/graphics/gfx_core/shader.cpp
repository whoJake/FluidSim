#include "shader.h"

namespace gfx
{

void shader_resource::initialise(shader_hash_t hash, shader_resource_type type, shader_stage_flags stages, u32 slot_index, u32 array_size, u32 resource_size)
{
    m_hash = hash;
    m_type = type;
    m_stages = stages;
    m_slotIndex = slot_index;
    m_arraySize = array_size;
    m_resourceSize = resource_size;
}

shader_hash_t shader_resource::get_hash() const
{
    return m_hash;
}

shader_resource_type shader_resource::get_type() const
{
    return m_type;
}

shader_stage_flags shader_resource::get_stages() const
{
    return m_stages;
}

u32 shader_resource::get_slot_index() const
{
    return m_slotIndex;
}

u32 shader_resource::get_array_size() const
{
    return m_arraySize;
}

u32 shader_resource::get_resource_size() const
{
    return m_resourceSize;
}

void shader_resource_group::initialise(u32 group_index, std::vector<shader_resource>&& resources)
{
    m_index = group_index;
    m_resources = std::move(resources);
    std::sort(
        m_resources.begin(),
        m_resources.end(),
        [](const shader_resource& left, const shader_resource& right)
        {
            return left.get_slot_index() < right.get_slot_index();
        });

    GFX_ASSERT(validate_resources(), "Shader resources are missing from resource group initialisation.");
}

u32 shader_resource_group::get_index() const
{

    return m_index;
}

const shader_resource& shader_resource_group::get_slot(u32 slot_index) const
{
    return m_resources[slot_index];
}

bool shader_resource_group::validate_resources() const
{
    for( u32 i = 0; i < u32_cast(m_resources.size()); i++ )
    {
        if( m_resources[i].get_slot_index() != i )
        {
            return false;
        }
    }
    return true;
}

void shader_stage::initialise(const char* entry_point, std::vector<shader_resource_group>&& groups, shader_stage_flags stage_flags)
{
    GFX_ASSERT(stage_flags, "Shader stage must have atleast one stage flag.");
    GFX_ASSERT(entry_point, "Shader stage must have an entry point.");

    m_entryPoint = entry_point;
    m_groups = std::move(groups);
    std::sort(
        m_groups.begin(),
        m_groups.end(),
        [](const shader_resource_group& left, const shader_resource_group& right)
        {
            return left.get_index() < right.get_index();
        });
    GFX_ASSERT(validate_groups(), "Shader resource groups are missing from shader stage initialisation.");
}

const char* shader_stage::get_entry_point() const
{
    return m_entryPoint;
}

shader_stage_flags shader_stage::get_stage_flags() const
{
    return m_stages;
}

bool shader_stage::validate_groups() const
{
    for( u32 i = 0; i < u32_cast(m_groups.size()); i++ )
    {
        if( m_groups[i].get_index() != i )
        {
            return false;
        }
    }
    return true;
}

void shader::initialise(shader_hash_t hash, std::vector<shader_stage>&& stages, void* pImpl)
{
    m_pImpl = pImpl;
    m_hash = hash;
    m_stages = std::move(stages);

    GFX_ASSERT(validate_stages(), "Shader stages are invalid.");
}

shader_hash_t shader::get_hash() const
{
    return m_hash;
}

const std::vector<shader_stage>& shader::get_stages() const
{
    return m_stages;
}

bool shader::validate_stages() const
{
    bool retval = true;
    shader_stage_flags usedStages{ 0 };
    for( const shader_stage& stage : m_stages )
    {
        if( usedStages & stage.get_stage_flags() )
        {
            GFX_ERROR("Stage flags {} already exist in shader. Shader must only have one of each shader stage type.", stage.get_stage_flags());
            retval = false;
        }
        else
        {
            usedStages |= stage.get_stage_flags();
        }
    }
    return retval;
}

} // gfx