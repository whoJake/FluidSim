#pragma once

// TODO these should be somewhere else
#define GFX_SUPPORTED 1
#define GFX_SUPPORTS_VULKAN

#if GFX_SUPPORTED

#include "device.h"

namespace gfx
{

#define GFX_CALL(func, ...) gfx::Driver::get_device()->func(__VA_ARGS__)

enum class DriverMode
{
#ifdef GFX_SUPPORTS_VULKAN
    vulkan,
#endif
};

class Driver
{
public:
    static u32 initialise(DriverMode mode);
    static u32 initialise(DriverMode mode, surface_create_func surface_func);
    static void shutdown();

    static device* get_device();
private:
    static device* sm_device;
    static std::function<u32(const std::vector<gpu>&)> sm_gpuSelector;
};

} // gfx

#endif