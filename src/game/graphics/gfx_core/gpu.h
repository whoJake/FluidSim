#pragma once
#include "gfxdefines.h"

namespace gfx
{

class gpu
{
public:
    gpu() = default;
    ~gpu() = default;

    DEFAULT_COPY(gpu);
    DEFAULT_MOVE(gpu);

    void initialise(const char* name,
                    u32 index,
                    u64 availableMemory,
                    bool isDedicated,
                    void* pImpl);

    bool is_dedicated() const;
    u64 get_total_memory() const;

    GFX_HAS_IMPL(m_pImpl);
private:
    const char* m_name;
    void* m_pImpl;
    u64 m_memory;
    u32 m_index;
    bool m_dedicated;
};

} // gfx