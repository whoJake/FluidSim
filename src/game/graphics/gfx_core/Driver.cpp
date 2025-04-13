#include "driver.h"

#ifdef GFX_SUPPORTS_VULKAN
    #include "vulkan/device_vk.h"
#endif

namespace gfx
{

u32 default_choose_gpu(const std::vector<gpu>&);

device* driver::sm_device = nullptr;
std::function<u32(const std::vector<gpu>&)> driver::sm_gpuSelector = default_choose_gpu;

MAKEPARAM(disable_debugger);


u32 driver::initialise(driver_mode mode)
{


#ifdef GFX_SUPPORTS_VULKAN
    return driver::initialise(
        mode,
        [](VkInstance, VkSurfaceKHR* surface) -> bool
        {
            *surface = VK_NULL_HANDLE;
            return true;
        });
#endif // GFX_SUPPORTS_VULKAN
}

u32 driver::initialise(driver_mode mode, surface_create_func surface_func)
{
    GFX_ASSERT(!sm_device, "Initialising graphics driver when one has already been initialised.");

    void* surface = nullptr;
    switch( mode )
    {
    #ifdef GFX_SUPPORTS_VULKAN
        case DRIVER_MODE_VULKAN:
        {
        #if GFX_VIRTUAL_DEVICE
            GFX_INFO("Creating virtual vulkan graphics device");
            sm_device = new device_vk();
        #else
            GFX_INFO("Creating vulkan graphics device");
            sm_device = new device();
        #endif
            break;
        }
    #endif // GFX_SUPPORTS_VULKAN
    }

    GFX_ASSERT(sm_device, "Graphics device initialisation failed.");

    sm_device->get_debugger().m_enabled = !p_disable_debugger.get();

    // DO NOT SUBMIT
    // gpuIdx = sm_gpuSelector(sm_device->enumerate_gpus());
    return sm_device->initialise(0, surface_func);
}

void driver::shutdown()
{
    sm_device->shutdown();
    delete sm_device;
    sm_device = nullptr;
}

void driver::map_resource(resource* resource)
{
    if( resource->is_mapped() )
        return;

    resource->m_mapped = GFX_CALL(map_resource, resource);
}

void driver::unmap_resource(resource* resource)
{
    if( !resource->is_mapped() )
        return;

    GFX_CALL(unmap_resource, resource);
    resource->m_mapped = nullptr;
}

void driver::create_buffer(buffer* buffer, const memory_info& memory_info)
{
    GFX_ASSERT(buffer, "Buffer cannot be nullptr.");
    GFX_ASSERT(buffer->get_backing_memory<void*>() == nullptr, "Buffer has already been allocated.");
    GFX_ASSERT(buffer->m_pImpl == nullptr, "Buffer has already been created.");

    buffer->m_pImpl = GFX_CALL(allocate_buffer, static_cast<resource*>(buffer), memory_info);

    if( memory_info.has_initial_data() )
        fill_initial_data(static_cast<resource*>(buffer), memory_info);
}

void driver::destroy_buffer(buffer* buffer)
{
    GFX_ASSERT(buffer, "Buffer cannot be nullptr.");
    GFX_ASSERT(!buffer->is_mapped(), "Buffer should be unmapped before destroying.");

    GFX_CALL(free_buffer, buffer);
    buffer->m_pImpl = nullptr;
    buffer->m_backingMemory = nullptr;
}

void driver::create_buffer_view(buffer_view* view, const buffer* buffer, buffer_view_range range, format format, resource_view_type type)
{
    GFX_ASSERT(view, "Buffer view cannot be nullptr.");
    GFX_ASSERT(buffer, "Buffer cannot be nullptr.");
    GFX_ASSERT(buffer->get_impl<void*>(), "Buffer must have been created.");
    GFX_ASSERT(type != RESOURCE_VIEW_INHERIT, "Buffer view cannot have view type inherit.");

    view->m_pResource = buffer;
    view->m_format = format;
    view->m_type = type;
    view->m_pImpl = GFX_CALL(create_buffer_view_impl, view, range);
}

void driver::destroy_buffer_view(buffer_view* view)
{
    GFX_ASSERT(view, "Buffer view cannot be nullptr.");
    GFX_ASSERT(view->get_impl<void*>(), "Buffer view is not created.");

    GFX_CALL(destroy_buffer_view_impl, view);
    view->m_pImpl = nullptr;
}

void driver::create_texture(texture* texture, const memory_info& memory_info, resource_view_type view_type)
{
    GFX_ASSERT(texture, "Texture cannot be nullptr.");
    GFX_ASSERT(texture->get_backing_memory<void*>() == nullptr, "Texture has already been allocated.");
    GFX_ASSERT(texture->m_pImpl == nullptr, "Texture has already been created.");

    texture->m_pImpl = GFX_CALL(allocate_texture, texture, memory_info, view_type);

    if( memory_info.has_initial_data() )
        fill_initial_data(static_cast<resource*>(texture), memory_info);
}

void driver::create_swapchain_texture(texture* texture, const memory_info& memory_info, void* pImpl)
{
    texture->m_pImpl = pImpl;
    texture->m_layout = texture_layout::TEXTURE_LAYOUT_UNDEFINED;
    texture->m_isSwapchain = true;
}

void driver::destroy_texture(texture* texture)
{
    GFX_ASSERT(texture, "Texture cannot be nullptr.");
    GFX_ASSERT(!texture->is_mapped(), "Texture should be unmapped before destroying.");

    GFX_CALL(free_texture, texture);
    texture->m_pImpl = nullptr;
    texture->m_backingMemory = nullptr;
}

void driver::create_texture_view(texture_view* view, const texture* texture, texture_view_range range, format format, resource_view_type type)
{
    GFX_ASSERT(view, "Texture view cannot be nullptr.");
    GFX_ASSERT(texture, "Texture cannot be nullptr.");
    GFX_ASSERT(texture->get_impl<void*>(), "Texture is not created.");

    view->m_pResource = texture;
    view->m_format = format;
    view->m_type = type;
    view->m_pImpl = GFX_CALL(create_texture_view_impl, view, range);
}

void driver::destroy_texture_view(texture_view* view)
{
    GFX_ASSERT(view, "Texture view cannot be nullptr.");
    GFX_ASSERT(view->get_impl<void*>(), "Texture view is not created.");

    GFX_CALL(destroy_texture_view_impl, view);
    view->m_pImpl = nullptr;
}

void driver::fill_initial_data(resource* resource, const memory_info& memory_info)
{
    GFX_ASSERT(resource, "Resource must not be nullptr.");
    GFX_ASSERT(memory_info.has_initial_data(), "Trying to call fill initial data without initial data.");
    // TODO do staging automatically?

    map_resource(resource);
    memcpy(resource->get_mapped(), memory_info.get_initial_data(), memory_info.get_size());
    unmap_resource(resource);
}

void driver::wait_idle()
{
    GFX_CALL(wait_idle);
}

screen_capabilities driver::query_screen_capabilities()
{
    return GFX_CALL(query_screen_capabilities);
}

device* driver::get_device()
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