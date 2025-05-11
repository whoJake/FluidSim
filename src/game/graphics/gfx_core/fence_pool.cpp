#include "fence_pool.h"
#include "Driver.h"

namespace gfx
{

void fence_pool::shutdown()
{
    GFX_ASSERT(m_fences, "Fence pool hasn't been initialised.");
    GFX_ASSERT(m_activeFences == 0, "Fence pool has not been fully reset.");

    for( u32 i = 0; i < m_capacity; i++ )
    {
        GFX_CALL(free_fence, &m_fences[i]);
    }
}

void fence_pool::initialise(u32 count)
{
    GFX_ASSERT(!m_fences, "Fence pool is already initialised.");

    m_fences = new fence[count];
    m_activeFences = 0;
    m_capacity = 0;

    for( u32 i = 0; i < count; i++ )
    {
        m_fences[i] = GFX_CALL(create_fence, false);
    }
}

fence* fence_pool::request_fence()
{
    if( m_activeFences == m_capacity )
    {
        return nullptr;
    }

    return &m_fences[++m_activeFences];
}

bool fence_pool::wait(u64 timeout)
{
    return GFX_CALL(wait_for_fences, m_fences, m_activeFences, timeout, true);
}

bool fence_pool::reset()
{
    if( !GFX_CALL(reset_fences, m_fences, m_activeFences) )
        return false;

    m_activeFences = 0;
    return true;
}

} // gfx
