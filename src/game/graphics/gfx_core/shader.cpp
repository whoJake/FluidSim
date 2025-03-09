#include "shader.h"
#include "Driver.h"
#include "system/hash.h"

namespace gfx
{

dt::hash_string32 program::get_name() const
{
    return m_name;
}

pass& program::get_pass(u64 index)
{
    return m_passes[index];
}

const pass& program::get_pass(u64 index) const
{
    return m_passes[index];
}

shader& program::get_shader(u64 index)
{
    return m_shaders[index];
}

const shader& program::get_shader(u64 index) const
{
    return m_shaders[index];
}

u64 program::get_pass_count() const
{
    return m_passes.size();
}

u64 program::get_shader_count() const
{
    return m_shaders.size();
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

descriptor_table_desc* pass::get_descriptor_table(descriptor_table_type type)
{
    return m_tables[type];
}

void pass::set_descriptor_table(descriptor_table_desc* desc, descriptor_table_type type)
{
    m_tables[type] = desc;
}

const descriptor_table_desc* pass::get_descriptor_table(descriptor_table_type type) const
{
    return m_tables[type];
}

u32 pass::get_descriptor_table_count() const
{
    return m_tableCount;
}

const pipeline_state& pass::get_pipeline_state() const
{
    return m_pso;
}

const shader_pass_outputs& pass::get_outputs() const
{
    return m_outputs;
}

void pass::set_impl(void* pImpl)
{
    m_pImpl = pImpl;
}

void pass::set_layout_impl(void* pImpl)
{
    m_pLayoutImpl = pImpl;
}

void shader::initialise(shader_stage_flag_bits stage)
{
    m_stage = stage;
}

shader_stage_flag_bits shader::get_stage() const
{
    return m_stage;
}

const dt::array<u32>& shader::get_code() const
{
    return m_code;
}

const char* shader::get_entry_point() const
{
    return m_entryPoint.try_get_str().data();
}

void shader::clear_code()
{
    m_code = dt::array<u32>(0);
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

void descriptor_table_desc::initialise(const dt::vector<descriptor_slot_desc>& buffer_slots, const dt::vector<descriptor_slot_desc>& image_slots)
{
    m_bufferDescs.initialise(buffer_slots.size(), false);
    m_imageDescs.initialise(image_slots.size(), false);
    m_lookup.reserve(buffer_slots.size() + image_slots.size());

    auto insert_slot_cache = [&](slot_cache&& slot) -> void
        {
            auto it = std::lower_bound(
                m_lookup.cbegin(),
                m_lookup.cend(),
                slot,
                [](const slot_cache& left, const slot_cache& right)
                {
                    return left.name.get_hash() < right.name.get_hash();
                });

            bool found = !(it == m_lookup.cend() || it->name.get_hash() != slot.name.get_hash());
            GFX_ASSERT(!found, "Descriptor slot {} ({}) has already been added to descriptor table.", slot.name.get_hash(), slot.name.try_get_str());

            m_lookup.insert(m_lookup.index_of(it), std::move(slot));
        };

    for( u64 idx = 0; idx < buffer_slots.size(); idx++ )
    {
        m_bufferDescs[idx] = buffer_slots[idx];

        slot_cache slot{
            buffer_slots[idx].get_name(),
            u32_cast(idx)
        };

        insert_slot_cache(std::move(slot));
    }

    for( u64 idx = 0; idx < image_slots.size(); idx++ )
    {
        m_imageDescs[idx] = image_slots[idx];

        slot_cache slot{
            buffer_slots[idx].get_name(),
            u32_cast(m_bufferDescs.size() + idx)
        };

        insert_slot_cache(std::move(slot));
    }
}

u64 descriptor_table_desc::find_buffer_slot(dt::hash_string32 name) const
{
    auto it = std::lower_bound(
        m_lookup.begin(),
        m_lookup.end(),
        name,
        [](const slot_cache& slot, const dt::hash_string32& name)
        {
            return slot.name.get_hash() < name.get_hash();
        });

    GFX_ASSERT(it != m_lookup.end() && it->name.get_hash() == name.get_hash(), "Buffer descriptor {} ({}) was not found inside descriptor table.", name.get_hash(), name.try_get_str());
    GFX_ASSERT(it->index < m_bufferDescs.size(), "Descriptor {} ({}) is not a buffer slot.", name.get_hash(), name.try_get_str());
    return it->index;
}

u64 descriptor_table_desc::find_image_slot(dt::hash_string32 name) const
{
    auto it = std::lower_bound(
        m_lookup.begin(),
        m_lookup.end(),
        name,
        [](const slot_cache& slot, const dt::hash_string32& name)
        {
            return slot.name.get_hash() < name.get_hash();
        });

    GFX_ASSERT(it != m_lookup.end() && it->name.get_hash() == name.get_hash(), "Buffer descriptor {} ({}) was not found inside descriptor table.", name.get_hash(), name.try_get_str());
    GFX_ASSERT(it->index >= m_bufferDescs.size(), "Descriptor {} ({}) is not a buffer slot.", name.get_hash(), name.try_get_str());
    return it->index - m_bufferDescs.size();
}

u64 descriptor_table_desc::calculate_hash() const
{
    u64 seed = 0x9ae16a3b2f90404fULL; // TODO: is this good?

    auto hash_slot_desc = [](const descriptor_slot_desc& slot) -> u64
        {
            u64 seed = 0x9ae16a3b2f90404fULL; // TODO: ??
            sys::combine_hash64_value(seed, slot.get_name().get_hash());
            sys::combine_hash64_value(seed, u64_cast(slot.get_resource_type()));
            sys::combine_hash64_value(seed, slot.get_array_size());
            sys::combine_hash64_value(seed, slot.get_slot_size());
            sys::combine_hash64_value(seed, slot.get_resource_size());
            sys::combine_hash64_value(seed, u64_cast(slot.get_visibility()));
            return seed;
        };

    for( const descriptor_slot_desc& slot : m_bufferDescs )
    {
        sys::combine_hash64_value(seed, hash_slot_desc(slot));
    }

    for( const descriptor_slot_desc& slot : m_imageDescs )
    {
        sys::combine_hash64_value(seed, hash_slot_desc(slot));
    }

    return seed;
}

const dt::array<descriptor_slot_desc>& descriptor_table_desc::get_buffer_descriptions() const
{
    return m_bufferDescs;
}

const dt::array<descriptor_slot_desc>& descriptor_table_desc::get_image_descriptions() const
{
    return m_imageDescs;
}

void descriptor_table_desc::set_impl(void* pImpl)
{
    m_pImpl = pImpl;
}

} // gfx