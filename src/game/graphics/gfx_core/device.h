#pragma once

#include "gfxdefines.h"
#include "gpu.h"
#include "debugger.h"
#include "buffer.h"

namespace gfx
{

class device
{
public:
    device() = default;
    virtual ~device() = default;

    DELETE_COPY(device);
    DELETE_MOVE(device);

    virtual u32 initialise(u32 gpuIdx, void* surface) = 0;
    virtual u32 initialise(u32 gpuIdx) = 0;
    virtual void shutdown() = 0;

    virtual std::vector<gpu> enumerate_gpus() const = 0;

    virtual buffer create_buffer(u64 size, buffer_usage usage, memory_type mem_type, bool persistant) = 0;
    virtual void map(buffer* buf) = 0;
    virtual void unmap(buffer* buf) = 0;
    virtual void free_buffer(buffer* buf) = 0;

    virtual void wait_idle() = 0;

    inline debugger& get_debugger()
    {
        return m_debugger;
    }

    virtual void dump_info() const = 0;
protected:
    debugger m_debugger{ };
    void* m_surface{ };
};

} // gfx