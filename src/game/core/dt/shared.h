#pragma once
#include "datatype_channel.h"
#include "system/allocator.h"

namespace dt
{

class default_allocator
{
public:
    static void* allocate(u64 size, u64 align)
    {
        return sys::allocator::allocate(size, align);
    }

    static void free(void* ptr, u64 size)
    { 
        sys::allocator::free(ptr, size);
    }
};

} // dt