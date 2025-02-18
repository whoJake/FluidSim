#include "shader.h"

#include "Driver.h"

namespace gfx
{

dt::hash_string32 program::get_name() const
{
    return m_name;
}

const pass& program::get_pass(u64 index) const
{
    return m_passes[index];
}

u64 program::get_pass_count() const
{
    return m_passes.size();
}

shader_stage_flags pass::get_stage_mask() const
{
    return m_stageMask;
}

u8 pass::get_vertex_shader_index() const
{
    return m_vertexShaderIndex;
}

u8 pass::get_geometry_shader_index() const
{
    return m_geometryShaderIndex;
}


u8 pass::get_fragment_shader_index() const
{
    return m_fragmentShaderIndex;
}


u8 pass::get_compute_shader_index() const
{
    return m_computeShaderIndex;
}

const descriptor_table_desc& pass::get_descriptor_table(descriptor_table_type type) const
{
    return m_tables[type];
}

const pipeline_state& pass::get_pipeline_state() const
{
    return m_pso;
}

void shader::initialise(shader_stage_flag_bits stage)
{
    m_stage = stage;
}

void descriptor_slot_desc::initialise(dt::hash_string32 name, shader_resource_type type, u32 array_size, u32 slot_size, u32 resource_size, shader_stage_flags visibility)
{
    m_name = name;
    m_type = type;
    m_arraySize = array_size;
    m_slotSize = slot_size;
    m_resourceSize = resource_size;
    m_visibility = visibility;
}

dt::hash_string32 descriptor_slot_desc::get_name() const
{
    return m_name;
}

shader_resource_type descriptor_slot_desc::get_resource_type() const
{
    return m_type;
}

u32 descriptor_slot_desc::get_array_size() const
{
    return m_arraySize;
}

u32 descriptor_slot_desc::get_slot_size() const
{
    return m_slotSize;
}

u32 descriptor_slot_desc::get_resource_size() const
{
    return m_resourceSize;
}

shader_stage_flags descriptor_slot_desc::get_visibility() const
{
    return m_visibility;
}

u64 descriptor_table_desc::find_buffer_slot(dt::hash_string32 name) const
{
    auto it = std::lower_bound(
        m_bufferDescs.begin(),
        m_bufferDescs.end(),
        name,
        [](const descriptor_slot_desc& slot, const dt::hash_string32& name)
        {
            return slot.get_name().get_hash() < name.get_hash();
        });

    GFX_ASSERT(it != m_bufferDescs.end() && it->get_name().get_hash() == name.get_hash(), "Buffer descriptor {} ({}) was not found inside descriptor table.", name.get_hash(), name.try_get_str());
    return m_bufferDescs.index_of(it);
}

u64 descriptor_table_desc::find_image_slot(dt::hash_string32 name) const
{
    auto it = std::lower_bound(
        m_imageDescs.begin(),
        m_imageDescs.end(),
        name,
        [](const descriptor_slot_desc& slot, const dt::hash_string32& name)
        {
            return slot.get_name().get_hash() < name.get_hash();
        });

    GFX_ASSERT(it != m_imageDescs.end() && it->get_name().get_hash() == name.get_hash(), "Buffer descriptor {} ({}) was not found inside descriptor table.", name.get_hash(), name.try_get_str());
    return m_imageDescs.index_of(it);
}

const dt::array<descriptor_slot_desc>& descriptor_table_desc::get_buffer_descriptions() const
{
    return m_bufferDescs;
}

const dt::array<descriptor_slot_desc>& descriptor_table_desc::get_image_descriptions() const
{
    return m_imageDescs;
}

void descriptor_table::initialise(descriptor_table_desc* owner, void* pImpl)
{
    GFX_ASSERT(owner, "Descriptor table must be created with an owner.");
    GFX_ASSERT(pImpl, "Descriptor table must have a pre-computed impl pointer.");

    m_desc = owner;
    m_bufferViews = dt::array<void*>(owner->get_buffer_descriptions().size());
    m_imageViews = dt::array<void*>(owner->get_image_descriptions().size());
    m_pImpl = pImpl;
}

void descriptor_table::set_buffer(dt::hash_string32 name, void* value)
{
    m_bufferViews[m_desc->find_buffer_slot(name)] = value;
}

void descriptor_table::set_image(dt::hash_string32 name, void* value)
{
    m_imageViews[m_desc->find_image_slot(name)] = value;
}

const dt::array<void*>& descriptor_table::get_buffer_views() const
{
    return m_bufferViews;
}

const dt::array<void*>& descriptor_table::get_image_views() const
{
    return m_imageViews;
}

void descriptor_table::write()
{
    GFX_CALL(write_descriptor_table, this);
}

} // gfx