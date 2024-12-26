#include "fence_pool.h"
#include "Driver.h"

namespace gfx
{

fence_pool::fence_pool(u32 count) :
    m_fences(new fence[count]),
    m_activeFences(0),
    m_capacity(count)
{
    for( u32 i = 0; i < count; i++ )
    {
        m_fences[i] = GFX_CALL(create_fence, false);
    }
}

fence_pool::~fence_pool()
{
    // TODO assert active_fences = 0

    for( u32 i = 0; i < m_capacity; i++ )
    {
        GFX_CALL(free_fence, &m_fences[i]);
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
