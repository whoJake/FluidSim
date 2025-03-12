#pragma once
#include "system_channel.h"

namespace sys
{

class allocator
{
public:
    static constexpr u64 minimum_align = 1uLL;
    static constexpr u64 default_align = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
    static constexpr u64 max_reasonable_allocate = 4_GiB;

    virtual ~allocator() { };

    static void* allocate(u64 size, u64 align);
    static void free(void* ptr, u64 size);

    virtual void* do_allocate(u64 size, u64 align) = 0;
    virtual void do_free(void* ptr, u64 size) = 0;
protected:
    allocator() = default;
public:
    // Static interface
    static allocator* const get_underlying_allocator();
    static void set_underlying_allocator(allocator* ptr);
private:
    static allocator** get_underlying_internal();
};

} // sys