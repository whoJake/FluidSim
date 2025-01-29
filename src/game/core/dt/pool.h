#pragma once
#include "system/allocator.h"
#include "system/tracked_allocator.h"

namespace dt
{

class pool_ami
{
public:
    virtual void* allocate(u64 size, u64 align) = 0;
    virtual void free(void* ptr) = 0;
};

template<sys::memory_zone zone>
class pool_am : public pool_ami
{
public:
    static inline pool_am* get()
    {
        static pool_am instance;
        return &instance;
    }

    inline void* allocate(u64 size, u64 align) override
    {
        SYSUSE_ZONE(zone);
        return sys::allocator::get_main()->allocate(size, align);
    }

    inline void free(void* ptr) override
    {
        SYSUSE_ZONE(zone);
        return sys::allocator::get_main()->free(ptr);
    }
};

using default_pool_am = pool_am<sys::MEMZONE_POOL>;

class pool_base
{
public:
    pool_base(u32 capacity, u32 elem_size, u32 alignment, pool_ami* alloc_method);
    ~pool_base();

protected:
    u32 get_index(void* ptr) const;
    void* get_elem(u32 index) const;

    void* allocate();
    void free(void* ptr);

    bool is_in_pool(void* ptr) const;

    void handle_oom();
private:
    void initialise_free_list();
    void kill();
private:
    u8* m_data;
    u8* m_firstFree;
    u32 m_freeCount;
    u32 m_capacity;
    u32 m_elemSize;

    pool_ami* m_method;
};

template<typename T>
class pool : public pool_base
{
public:
    pool(u32 capacity, pool_ami* alloc_method = default_pool_am::get()) :
        pool_base(capacity, sizeof(T), alignof(T), alloc_method)
    { }

    inline u32 get_index(T* ptr) const
    {
        return pool_base::get_index(ptr);
    }

    inline T* get_elem(u32 index) const
    {
        return static_cast<T*>(pool_base::get_elem(index));
    }

    inline T* allocate()
    {
        return static_cast<T*>(pool_base::allocate());
    }

    inline void free(T* ptr)
    {
        pool_base::free(ptr);
    }

    inline bool is_in_pool(T* ptr) const
    {
        return pool_base::is_in_pool(ptr);
    }
};

template<typename T, sys::memory_zone zone>
class zoned_pool : public pool<T>
{
public:
    zoned_pool(u32 capacity) :
        pool<T>(capacity, pool_am<zone>::get())
    { };
};

} // dt