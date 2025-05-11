#include "memory.h"
#include "allocator.h"
#include "zone_allocator.h"

namespace sys
{

void memory::setup_heap()
{
    allocator::set_underlying_allocator(zone_allocator::get());
}

void memory::initialise_system_zones()
{
    zone_allocator::register_zone("system.default", MEMZONE_DEFAULT, { 0, budget_failure_type::silent });
    zone_allocator::register_zone("system.hashstring", MEMZONE_SYSTEM_HASHSTRING, { 0, budget_failure_type::silent });
}

void* memory::operator_new(u64 size, u64 align)
{
    return allocator::allocate(size, align);
}

void memory::operator_delete(void* ptr, u64 size)
{
    allocator::free(ptr, size);
}

namespace
{
struct array_header
{
    u64 size : 58;
    u64 align : 6;
};
}

void* memory::operator_new_arr(u64 size, u64 align)
{
    // Prepare for some cursed shit.
    // first 6 bytes of u64 before pointer we return is going to be alignment in bytes
    // final 58 bytes can be for the size, nothing can actually be bigger than 2^58 on modern
    // systems anyway right...

    // Alignment is essentially the extra size we have to allocate
    u64 alignment = std::max(alignof(u64), align);
    u64 actual_size = size + alignment;

    void* raw_ptr = allocator::allocate(actual_size, alignment);
    void* ret_ptr = (void*)((u64)raw_ptr + alignment);
     
    // Header is the immediate 8 bytes before the ret_ptr
    array_header* header = (array_header*)((u64)ret_ptr - sizeof(u64));
    header->size = actual_size;
    header->align = alignment;

    return ret_ptr;
}

void memory::operator_delete_arr(void* ptr)
{
    if( ptr == nullptr ) [[unlikely]]
    {
        allocator::free(ptr, 0);
        return;
    }

    array_header* header = (array_header*)((u64)ptr - sizeof(u64));
    u64 size = header->size;
    u64 align = header->align;

    MEM_ASSERT(align >= alignof(u64), "Corrupted array pointer.");

    void* raw_ptr = (void*)((u64)ptr - align);
    allocator::free(raw_ptr, size);
}

} // sys