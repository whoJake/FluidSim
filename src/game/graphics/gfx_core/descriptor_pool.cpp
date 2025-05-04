#include "descriptor_pool.h"
#include "Driver.h"

namespace gfx
{

void descriptor_pool::initialise(descriptor_table_desc* desc, u32 size, bool reuse_tables)
{
    m_desc = desc;
    m_capacity = size;
    m_reuseTables = reuse_tables;

    m_pImpl = GFX_CALL(create_descriptor_pool_impl, desc, size, reuse_tables);
}

descriptor_table* descriptor_pool::allocate()
{
    if( u32_cast(m_used.size()) >= m_capacity )
        return nullptr;

    if( m_available.size() > 0 )
        return take_available();

    return allocate_from_pool();
}

const descriptor_table_desc& descriptor_pool::get_table_desc() const
{
    return *m_desc;
}

descriptor_table* descriptor_pool::take_available()
{
    GFX_ASSERT(m_available.size() > 0, "Trying to take available descriptor table but there aren't any. Are there multiple threads accessing this pool?");

    // This is a weird way to go about it but I cba to implement pop_back right now
    descriptor_table* tmp = m_available.back().release();
    m_available.erase(--m_available.end());
    std::unique_ptr<descriptor_table> temp(tmp);
    m_used.push_back(std::move(temp));
    return tmp;
}

descriptor_table* descriptor_pool::allocate_from_pool()
{
    GFX_ASSERT(m_available.size() == 0, "Trying to allocate a new descriptor from pool, but there are some available to reuse. Are there multiple threads accessing this pool?");
    GFX_ASSERT(u32_cast(m_used.size()) < m_capacity, "Trying to allocate a new descriptor from pool but theres no space for it. Are there multiple threads accessing this pool?");

    std::unique_ptr<descriptor_table> created = std::make_unique<descriptor_table>();

    void* pImpl = GFX_CALL(allocate_descriptor_table_impl, this);
    created->initialise(this, pImpl);

    descriptor_table* retval = created.get();
    m_used.push_back(std::move(created));
    return retval;
}

void descriptor_pool::free(descriptor_table* table)
{
    for( u64 idx = 0; idx < m_used.size(); idx++ )
    {
        if( m_used[idx].get() == table )
        {
            m_used[idx].release();
            m_used.erase(m_used.begin() + idx);

            if( m_reuseTables )
            {
                m_available.push_back(std::unique_ptr<descriptor_table>(table));
            }
            else
            {
                // TODO actually free the descriptor pool. This is only used for ImGui atm.
                delete table;
            }
            return;
        }
    }

    GFX_ASSERT(false, "Descriptor table is not a part of this descriptor pool.");
}

void descriptor_pool::soft_reset()
{
    for( u64 i = 0; i < m_used.size(); i++ )
    {
        descriptor_table* pTable = m_used[i].release();
        m_available.push_back(std::unique_ptr<descriptor_table>(pTable));
    }

    m_used.resize(0);
}

void descriptor_pool::reset()
{
    GFX_CALL(reset_descriptor_pool, this);

    // Should we assert that m_used is empty? Does it make sense for us to return descriptor sets
    // before we reset the pool? It shouldn't be too expensive since we never actually call vkFreeDescriptorSets
    m_used.resize(0);
    m_available.resize(0);
}

u32 descriptor_pool::get_capacity() const
{
    return m_capacity;
}

void descriptor_table::initialise(descriptor_pool* owner, void* pImpl)
{
    GFX_ASSERT(owner, "Descriptor table must be created with an owner.");
    GFX_ASSERT(pImpl, "Descriptor table must have a pre-computed impl pointer.");

    m_owner = owner;
    m_bufferViews = std::vector<buffer*>(get_desc().get_buffer_descriptions().size());
    m_imageViews = std::vector<void*>(get_desc().get_image_descriptions().size());
    m_pImpl = pImpl;
}

void descriptor_table::set_buffer(dt::hash_string32 name, buffer* value)
{
    m_bufferViews[get_desc().find_buffer_slot(name)] = value;
}

void descriptor_table::set_image(dt::hash_string32 name, void* value)
{
    m_imageViews[get_desc().find_image_slot(name)] = value;
}

const std::vector<buffer*>& descriptor_table::get_buffer_views() const
{
    return m_bufferViews;
}

const std::vector<void*>& descriptor_table::get_image_views() const
{
    return m_imageViews;
}

const descriptor_table_desc& descriptor_table::get_desc() const
{
    return m_owner->get_table_desc();
}

void descriptor_table::write()
{
    GFX_CALL(write_descriptor_table, this);
}

} // gfx