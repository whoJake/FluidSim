#include "pool.h"

namespace dt
{

pool_base::pool_base(u32 capacity, u32 elem_size, u32 alignment, allocator* alloc_method) :
    m_capacity(capacity),
    m_freeCount(capacity),
    m_method(alloc_method)
{
    u32 actual_elem_size = std::max(elem_size, u32_cast(sizeof(void*)));
    m_elemSize = actual_elem_size;
    m_data = static_cast<u8*>(m_method->allocate(u64_cast(capacity) * actual_elem_size, alignment));
    m_firstFree = m_data;
    initialise_free_list();
}

pool_base::~pool_base()
{
    kill();
}

u32 pool_base::get_index(void* ptr) const
{
    // SYSFATAL_ASSERT(is_in_pool(ptr), "ptr is not in pool.");
    return u32_cast((m_data - reinterpret_cast<u8*>(ptr)) / u64_cast(m_elemSize));
}

void* pool_base::get_elem(u32 index) const
{
    return m_data + (u64_cast(index) * m_elemSize);
}

void* pool_base::allocate()
{
    if( !m_freeCount )
    {
        handle_oom();
        return nullptr;
    }

    u8* nextFree = *reinterpret_cast<u8**>(m_firstFree);
    void* retval = reinterpret_cast<void*>(m_firstFree);
    m_firstFree = nextFree;
    m_freeCount--;

    return retval;
}

void pool_base::free(void* ptr)
{
    // SYSFATAL_ASSERT(is_in_pool(ptr), "ptr is not in pool.");
    *reinterpret_cast<u8**>(ptr) = m_firstFree;
    m_firstFree = reinterpret_cast<u8*>(ptr);
    m_freeCount++;
}

bool pool_base::is_in_pool(void* ptr) const
{
    return ptr > m_data && ptr < m_data + (u64_cast(m_capacity) * m_elemSize);
}

void pool_base::handle_oom()
{
    return;
}

void pool_base::initialise_free_list()
{
    for( u32 idx = 0; idx < m_capacity - 1; idx++ )
    {
        u8* cur = m_data + (idx * m_elemSize);
        u8* next = cur + m_elemSize;
        *reinterpret_cast<u8**>(cur) = next;
    }
}

void pool_base::kill()
{
    m_method->free(m_data);
}

} // dt