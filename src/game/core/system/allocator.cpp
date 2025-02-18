#include "allocator.h"
#include "basic_allocator.h"

namespace sys
{

// Default value.
allocator* allocator::sm_main = basic_allocator::get();

void* allocator::allocate(u64 size, u64 align)
{
    void* ptr = do_allocate(size, align);
    return ptr;
}

void allocator::free(void* ptr)
{
    if( !ptr ) [[unlikely]]
        return;

    do_free(ptr);
}

allocator* const allocator::get_main()
{
    return sm_main;
}

void allocator::set_main(allocator* ptr)
{
    MEMASSERT_FATAL(ptr, "Main allocator is already set.");
    sm_main = ptr;
}

} // sys