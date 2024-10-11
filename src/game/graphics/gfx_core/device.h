#pragma once

#include "gfxdefines.h"
#include "gpu.h"
#include "debugger.h"

namespace gfx
{

class device
{
public:
    device() = default;
    virtual ~device() = default;

    DELETE_COPY(device);
    DELETE_MOVE(device);

    virtual u32 initialise(u32 gpuIdx) = 0;
    virtual void shutdown() = 0;

    virtual std::vector<gpu> enumerate_gpus() const = 0;

    virtual void wait_idle() = 0;

    inline debugger& get_debugger()
    {
        return m_debugger;
    }
protected:
    debugger m_debugger{ };
};

} // gfx