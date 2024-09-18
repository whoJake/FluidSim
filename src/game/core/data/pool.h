#pragma once

#define MTL_POOL_ZERO_ON_ALLOCATE 1
#define MTL_POOL_CHECK_PTR_FREE 1

namespace mtl
{

template<typename T>
class pool
{
public:
    using type = T;
    using ptr_type = T*;
    using ref_type = T&;
    using const_ref_type = const T&;

    static constexpr u64 slot_size = sizeof(T) >= sizeof(void*)
                                    ? sizeof(T)
                                    : sizeof(void*);

    pool(u32 capacity) :
        m_data(new u8[capacity * slot_size]),
        m_firstFree(m_data),
        m_freeCount(capacity),
        m_capacity(capacity)
    {
        initialise_free_list();
    }

    ~pool()
    {
        delete[] m_data;
    }

    T* allocate()
    {
        // no space.
        if( !m_freeCount )
        {
            return nullptr;
        }

        u8* nextFree = *reinterpret_cast<u8**>(m_firstFree);
        T* retval = reinterpret_cast<T*>(m_firstFree);
        m_firstFree = nextFree;
        m_freeCount--;

    #if MTL_POOL_ZERO_ON_ALLOCATE
        memset(retval, 0, sizeof(T));
    #endif

        return retval;
    }

    void free(T* ptr)
    {
    #if MTL_POOL_CHECK_PTR_FREE
        assert(is_ptr_in_pool(ptr));
    #endif
        
        *reinterpret_cast<u8**>(ptr) = m_firstFree;
        m_firstFree = reinterpret_cast<u8*>(ptr);
        m_freeCount++;
    }

    u64 size() const
    {
        return u64_cast(m_capacity - m_freeCount);
    }

    void for_each(std::function<void(T*)> func)
    {
        if( !m_freeCount )
        {
            return;
        }

        // build a sorted free list
        std::vector<u8*> freed;
        freed.push_back(m_firstFree);
        while( u32_cast(freed.size()) != m_freeCount )
        {
            u8* lastFree = freed.back();
            u8* nextFree = *reinterpret_cast<u8**>(lastFree);
            freed.push_back(nextFree);
        }

        std::sort(freed.begin(), freed.end());

        // now we iterate our pool and skip any that are in our sorted free array
        u32 currFree = 0;
        for( u32 idx = 0; idx < m_capacity; idx++ )
        {
            u8* it = m_data + (idx * slot_size);
            if( currFree == m_freeCount )
            {
                // always do func
                func(reinterpret_cast<T*>(it));
            }
            else
            {
                // our iterator is in the freelist
                if( it == freed[currFree] )
                {
                    currFree++;
                    continue;
                }
                else
                {
                    func(reinterpret_cast<T*>(it));
                }
            }
        }
    }
private:
    void initialise_free_list()
    {
        for( u32 idx = 0; idx < m_capacity - 1; idx++ )
        {
            u8* current = m_data + (idx * slot_size);
            u8* next = current + slot_size;
            *reinterpret_cast<u8**>(current) = next;
        }
    }

    bool is_ptr_in_pool(T* ptr) const
    {
        u8* p = reinterpret_cast<u8*>(ptr);

        return p >= m_data 
            && p < (m_data + (slot_size * (m_capacity)));
    }

private:
    u8* m_data;
    u8* m_firstFree;
    u32 m_freeCount;
    u32 m_capacity;
};

} // mtl