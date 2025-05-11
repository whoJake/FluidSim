#include "gpu.h"

namespace gfx
{

void gpu::initialise(const char* name,
                     u32 index,
                     u64 availableMemory,
                     bool isDedicated,
                     void* pImpl)
{
    m_name = name;
    m_pImpl = pImpl;
    m_memory = availableMemory;
    m_index = index;
    m_dedicated = isDedicated;
}

bool gpu::is_dedicated() const
{
    return m_dedicated;
}

u64 gpu::get_total_memory() const
{
    return m_memory;
}

} // gfx