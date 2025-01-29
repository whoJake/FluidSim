#pragma once

#include "allocator.h"

namespace sys
{

class basic_allocator : public allocator
{
public:
    virtual void* do_allocate(u64 size, u64 align) override;
    virtual void do_free(void* ptr) override;
public:
    static basic_allocator* get();
};

} // sys