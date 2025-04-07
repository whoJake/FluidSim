#pragma once

#include "resource.h"
#include "types.h"
#include "gfxdefines.h"

namespace gfx
{

class texture_info
{
public:
    texture_info() = default;
    texture_info(u16 width, u16 height, u16 depth_or_layers, u16 mip_count);
    ~texture_info() = default;

    DEFAULT_MOVE(texture_info);
    DEFAULT_COPY(texture_info);

    void initialise(u16 width, u16 height, u16 depth_or_layers, u16 mip_count);

    u16 get_width() const;
    u16 get_height() const;
    u16 get_depth() const;
    u16 get_layer_count() const;
    u16 get_mip_count() const;
private:
    u16 m_width;
    u16 m_height;
    u16 m_depthOrLayers;
    u16 m_mipCount;
};

class texture :
    public resource,
    public texture_info
{
public:
    friend class driver;
    friend class command_list; // TODO TEMPORARY

    // Texture requires view_type to create.. ):
    static texture create(const memory_info& memory_info, const texture_info& texture_info, texture_layout layout, resource_view_type view_type);
    static void destroy(texture* texture);

    texture() = default;
    texture(const texture_info& info);
    ~texture() = default;

    DEFAULT_MOVE(texture);
    DEFAULT_COPY(texture); // TODO can this be deleted?

    texture_view create_view(format format, resource_view_type type, texture_view_range range) const;

    texture_layout get_layout() const;
    bool is_swapchain_owned() const;

    GFX_HAS_IMPL(m_pImpl);
private:
    void* m_pImpl;
    texture_layout m_layout;
    u32 m_isSwapchain : 1;
    u32 m_unused : 31;
};

} // gfx