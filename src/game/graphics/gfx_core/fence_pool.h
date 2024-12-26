#pragma once

#include "fence.h"
#include "data/pool.h"

namespace gfx
{

class fence_pool
{
public:
    fence_pool() = default;
    fence_pool(u32 count);

    ~fence_pool();

    fence* request_fence();
    bool wait(u64 timeout = u64_max);
    bool reset();
private:
    fence* m_fences;
    u32 m_activeFences;
    u32 m_capacity;
};

} // gfx