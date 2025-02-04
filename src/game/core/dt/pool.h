#pragma once
#include "common.h"

namespace dt
{

template<typename T, typename _allocator = default_allocator>
class pool
{
public:
    pool(u32 capacity);
    ~pool();

    u32 get_index(T* ptr) const;
    T* get_elem(u32 index) const;
    T* allocate();

    void free(T* ptr);
    bool is_in_pool(T* ptr) const;

private:
    void initialise_free_list();
    void kill();

    static constexpr u64 elem_size = std::max(sizeof(T), sizeof(void*));
private:
    u8* m_data;
    u8* m_firstFree;
    u32 m_freeCount;
    u32 m_capacity;
};

} // dt

#ifndef INC_DT_POOL_INL
    #define INC_DT_POOL_INL
    #include "pool.inl"
#endif