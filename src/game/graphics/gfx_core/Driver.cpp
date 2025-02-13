#include "Driver.h"
#include "system/tracked_allocator.h"

#ifdef GFX_SUPPORTS_VULKAN
    #include "vulkan/device_vk.h"
#endif

namespace gfx
{

SYSZONE_REGISTER(MEMZONE_SHADERS, MEMZONE_SHADERS);

u32 default_choose_gpu(const std::vector<gpu>&);

device* Driver::sm_device = nullptr;
std::function<u32(const std::vector<gpu>&)> Driver::sm_gpuSelector = default_choose_gpu;


u32 Driver::initialise(DriverMode mode)
{
#ifdef GFX_SUPPORTS_VULKAN
    return Driver::initialise(
        mode,
        [](VkInstance, VkSurfaceKHR* surface) -> bool
        {
            *surface = VK_NULL_HANDLE;
            return true;
        });
#endif // GFX_SUPPORTS_VULKAN
}

u32 Driver::initialise(DriverMode mode, surface_create_func surface_func)
{
    GFX_ASSERT(!sm_device, "Initialising graphics driver when one has already been initialised.");

    void* surface = nullptr;
    switch( mode )
    {
    #ifdef GFX_SUPPORTS_VULKAN
        case DriverMode::vulkan:
        {
            GFX_INFO("Creating vulkan graphics device");
            sm_device = new device_vk();
            device_vk* vkdevice = (device_vk*)sm_device;
            VkSurfaceKHR vkSurface = (VkSurfaceKHR)surface;
            bool surfaceSuccess = surface_func(vkdevice->get_impl_instance(), &vkSurface);
            surface = vkSurface;
            GFX_ASSERT(surfaceSuccess, "Failed to create surface for graphics device.");
            break;
        }
    #endif // GFX_SUPPORTS_VULKAN
    }

    GFX_ASSERT(sm_device, "Graphics device initialisation failed.");

    u32 gpuIdx = sm_gpuSelector(sm_device->enumerate_gpus());
    return sm_device->initialise(gpuIdx, surface);
}

void Driver::shutdown()
{
    sm_device->shutdown();
    delete sm_device;
    sm_device = nullptr;
}

device* Driver::get_device()
{
    return sm_device;
}

u32 default_choose_gpu(const std::vector<gpu>& options)
{
    u64 maxMemory = 0;
    u32 maxMemoryIdx = 0;

    for( u32 i = 0; i < u32_cast(options.size()); i++ )
    {
        if( options[i].is_dedicated() )
        {
            return i;
        }

        if( options[i].get_total_memory() > maxMemory )
        {
            maxMemoryIdx = i;
        }
    }

    return maxMemoryIdx;
}

} // gfx