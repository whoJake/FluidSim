#pragma once
#include "gfxdefines.h"

namespace gfx
{

class fence
{
public:
    fence(void* pImpl = nullptr);
    ~fence();

    bool wait(u64 timeout = u64_max) const;
    bool check() const;
    bool reset();

    GFX_HAS_IMPL(m_pImpl);
private:
    void* m_pImpl;
};

} // gfx