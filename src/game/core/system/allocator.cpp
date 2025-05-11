#include "allocator.h"
#include "basic_allocator.h"

namespace sys
{

void* allocator::allocate(u64 size, u64 align)
{
    MEM_ASSERT(align >= minimum_align, "Trying to allocate with alignment of {}. Minimum alignment is {}.", align, minimum_align);
    MEM_ASSERT(size <= max_reasonable_allocate, "Trying to allocate {} bytes. Max allocation is {}.", size, max_reasonable_allocate);

    void* ptr = get_underlying_allocator()->do_allocate(size, align);
    return ptr;
}

void allocator::free(void* ptr, u64 size)
{
    // freeing a nullptr is well defined so theres no need to check our pointer.
    get_underlying_allocator()->do_free(ptr, size);
}

allocator* const allocator::get_underlying_allocator()
{
    return *get_underlying_internal();
}

void allocator::set_underlying_allocator(allocator* ptr)
{
    *get_underlying_internal() = ptr;
}

allocator** allocator::get_underlying_internal()
{
    // Default allocator is basic_allocator, this means anytime you call allocate
    // or free it will return valid memory, even if the allocator is not set yet.
    static allocator* main = basic_allocator::get();
    return &main;
}

} // sys