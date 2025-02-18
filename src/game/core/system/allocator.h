#pragma once
#include "assert.h"

namespace sys
{

SYSDECLARE_CHANNEL(memory);

#define MEMASSERT_FATAL(cond, fmt, ...) SYSASSERT(cond, SYSMSG_CHANNEL_FATAL(memory, fmt, __VA_ARGS__))

class allocator
{
public:
    static constexpr u64 default_align = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

    ~allocator() = default;

    void* allocate(u64 size, u64 align = default_align);
    void free(void* ptr);

    virtual void* do_allocate(u64 size, u64 align) = 0;
    virtual void do_free(void* ptr) = 0;
protected:
    allocator() = default;
public:
    // Static interface
    static allocator* const get_main();
    static void set_main(allocator* ptr);
private:
    static allocator* sm_main;

};

} // sys