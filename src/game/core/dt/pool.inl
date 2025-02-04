#include "pool.h"

namespace dt
{

template<typename T, typename _allocator>
pool<T, _allocator>::pool(u32 capacity) :
    m_capacity(capacity),
    m_freeCount(capacity)
{
    m_data = static_cast<u8*>(_allocator::allocate(u64_cast(capacity) * elem_size, alignof(T)));
    m_firstFree = m_data;
    initialise_free_list();
}

template<typename T, typename _allocator>
pool<T, _allocator>::~pool()
{
    kill();
}

template<typename T, typename _allocator>
u32 pool<T, _allocator>::get_index(T* ptr) const
{
    DT_ASSERT(is_in_pool(ptr), "ptr is not in pool.");
    return u32_cast((m_data - reinterpret_cast<u8*>(ptr)) / elem_size);
}

template<typename T, typename _allocator>
T* pool<T, _allocator>::get_elem(u32 index) const
{
    return m_data + (index * elem_size);
}

template<typename T, typename _allocator>
T* pool<T, _allocator>::allocate()
{
    if( !m_freeCount )
    {
        return nullptr;
    }

    u8* nextFree = *reinterpret_cast<u8**>(m_firstFree);
    void* retval = reinterpret_cast<void*>(m_firstFree);
    m_firstFree = nextFree;
    m_freeCount--;

    return static_cast<T*>(retval);
}

template<typename T, typename _allocator>
void pool<T, _allocator>::free(T* ptr)
{
    DT_ASSERT(is_in_pool(ptr), "ptr is not in pool.");
    *reinterpret_cast<u8**>(ptr) = m_firstFree;
    m_firstFree = reinterpret_cast<u8*>(ptr);
    m_freeCount++;
}

template<typename T, typename _allocator>
bool pool<T, _allocator>::is_in_pool(T* ptr) const
{
    return ptr > m_data && ptr < m_data + (m_capacity * elem_size);
}

template<typename T, typename _allocator>
void pool<T, _allocator>::initialise_free_list()
{
    for( u32 idx = 0; idx < m_capacity - 1; idx++ )
    {
        u8* cur = m_data + (idx * elem_size);
        u8* next = cur + elem_size;
        *reinterpret_cast<u8**>(cur) = next;
    }
}

template<typename T, typename _allocator>
void pool<T, _allocator>::kill()
{
    _allocator::free(m_data);
}

} // dt