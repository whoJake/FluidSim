#include "context_mgr.h"

namespace gfx
{

void context_mgr::initialise()
{
    sm_instance = new context_mgr();
}

void context_mgr::shutdown()
{
    delete sm_instance;
}

} // gfx