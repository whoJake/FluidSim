#include "basic_allocator.h"

namespace sys
{

basic_allocator*  basic_allocator::get()
{
    static basic_allocator instance;
    return &instance;
}

void* basic_allocator::do_allocate(u64 size, u64 align)
{
    return _aligned_malloc(size, align);
}

void basic_allocator::do_free(void* ptr, u64 size)
{
    (void)size;
    _aligned_free(ptr);
}

} // sys