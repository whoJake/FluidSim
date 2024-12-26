#include "fence.h"
#include "Driver.h"

namespace gfx
{

fence::fence(void* pImpl) :
    m_impl(pImpl)
{ }

fence::~fence()
{ }

bool fence::wait(u64 timeout) const
{
    return GFX_CALL(wait_for_fences, this, 1, false, timeout);
}

bool fence::check() const
{
    return GFX_CALL(check_fence, this);
}

bool fence::reset()
{
    return GFX_CALL(reset_fences, this, 1);
}

} // gfx