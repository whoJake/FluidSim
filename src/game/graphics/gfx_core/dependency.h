#pragma once
#include "gfxdefines.h"

namespace gfx
{

class dependency
{
public:
    dependency() = default;
    ~dependency() = default;

    void initialise(void* pImpl, const char* debug_name = nullptr);

    GFX_HAS_IMPL(m_pImpl);
private:
    void* m_pImpl;
#if GFX_DEPENDENCY_NAMES
    const char* m_name;
#endif
};

} // gfx