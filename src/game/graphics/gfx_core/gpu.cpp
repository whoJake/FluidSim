#include "gpu.h"

namespace gfx
{

gpu::gpu(const char* name,
         u32 index,
         u64 availableMemory,
         bool isDedicated) :
    m_name(name),
    m_memory(availableMemory),
    m_index(index),
    m_dedicated(isDedicated)
{ }

bool gpu::is_dedicated() const
{
    return m_dedicated;
}

u64 gpu::get_total_memory() const
{
    return m_memory;
}

} // gfx