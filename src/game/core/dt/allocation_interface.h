#pragma once
#include "system/tracked_allocator.h"

namespace dt
{

class allocator
{
public:
    virtual void* allocate(u64 size, u64 align) = 0;
    virtual void free(void* ptr) = 0;
};

template<sys::memory_zone zone>
class zoned_allocator : public allocator
{
public:
    static inline zoned_allocator* get()
    {
        static zoned_allocator instance;
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

using default_allocator = zoned_allocator<sys::MEMZONE_DEFAULT>;

} // dt