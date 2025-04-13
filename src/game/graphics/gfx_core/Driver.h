#pragma once

// TODO these should be somewhere else
#define GFX_SUPPORTED 1
#define GFX_SUPPORTS_VULKAN

#if GFX_SUPPORTED

#include "device.h"

namespace gfx
{

#define GFX_CALL(func, ...) gfx::driver::get_device()->func(__VA_ARGS__)

enum driver_mode
{
#ifdef GFX_SUPPORTS_VULKAN
    DRIVER_MODE_VULKAN,
#endif
};

struct screen_capabilities;

class driver
{
public:
    static u32 initialise(driver_mode mode);
    static u32 initialise(driver_mode mode, surface_create_func surface_func);
    static void shutdown();

    static void create_buffer(buffer* buffer, const memory_info& memory_info);
    static void destroy_buffer(buffer* buffer);

    static void create_buffer_view(buffer_view* view, const buffer* buffer, buffer_view_range range, format format, resource_view_type type);
    static void destroy_buffer_view(buffer_view* view);

    static void create_texture(texture* texture, const memory_info& memory_info, resource_view_type view_type);
    static void create_swapchain_texture(texture* texture, const memory_info& memory_info, void* pImpl);
    static void destroy_texture(texture* texture);

    static void create_texture_view(texture_view* view, const texture* texture, texture_view_range range, format format, resource_view_type type);
    static void destroy_texture_view(texture_view* view);

    static void create_texture_sampler(texture_sampler* sampler, texture_view* view);
    static void destroy_texture_sampler(texture_sampler* sampler);

    static void map_resource(resource* resource);
    static void unmap_resource(resource* resource);

    static void wait_idle();
    static screen_capabilities query_screen_capabilities();

    static device* get_device();
private:
    static void fill_initial_data(resource* resource, const memory_info& memory_info);
private:
    static device* sm_device;
    static std::function<u32(const std::vector<gpu>&)> sm_gpuSelector;
};

struct screen_capabilities
{
    u32 min_image_count;
    u32 max_image_count;
    u32 current_width, current_height;
    u32 min_width, min_height;
    u32 max_width, max_height;
};

} // gfx

#endif