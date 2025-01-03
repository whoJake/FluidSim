#pragma once

#include "fence.h"
#include "data/pool.h"

namespace gfx
{

class fence_pool
{
public:
    fence_pool() = default;
    ~fence_pool() = default;

    void initialise(u32 count);
    void shutdown();

    fence* request_fence();
    bool wait(u64 timeout = u64_max);
    bool reset();
private:
    fence* m_fences;
    u32 m_activeFences;
    u32 m_capacity;
};

} // gfx