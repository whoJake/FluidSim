#include "dependency.h"

namespace gfx
{

void dependency::initialise(void* pImpl, const char* debug_name)
{
    m_pImpl = pImpl;
#if GFX_DEPENDENCY_NAMES
    m_name = debug_name;
#endif
}

} // gfx