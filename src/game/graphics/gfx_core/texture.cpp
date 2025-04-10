#include "texture.h"
#include "driver.h"

namespace gfx
{

texture_info::texture_info(u16 width, u16 height, u16 depth_or_layers, u16 mip_count)
{
    initialise(width, height, depth_or_layers, mip_count);
}

void texture_info::initialise(u16 width, u16 height, u16 depth_or_layers, u16 mip_count)
{
    m_width = width;
    m_height = height;
    m_depthOrLayers = depth_or_layers;
    m_mipCount = mip_count;
}

u16 texture_info::get_width() const
{
    return m_width;
}

u16 texture_info::get_height() const
{
    return m_height;
}

u16 texture_info::get_depth() const
{
    return m_depthOrLayers;
}

u16 texture_info::get_layer_count() const
{
    return m_depthOrLayers;
}

u16 texture_info::get_mip_count() const
{
    return m_mipCount;
}

texture texture::create(const memory_info& memory_info, const texture_info& texture_info, resource_view_type view_type)
{
    texture retval(texture_info);
    retval.m_isSwapchain = memory_info.get_texture_usage() & TEXTURE_USAGE_SWAPCHAIN_OWNED;
    driver::create_texture(&retval, memory_info, view_type);
    return retval;
}

void texture::destroy(texture* texture)
{
    if( !texture->is_swapchain_owned() )
        driver::destroy_texture(texture);
}

texture::texture(const texture_info& info) :
    resource(),
    texture_info(info),
    m_pImpl(nullptr),
    m_layout(texture_layout::TEXTURE_LAYOUT_UNDEFINED),
    m_isSwapchain(0),
    m_unused(0)
{ }

texture_view texture::create_view(format format, resource_view_type type, texture_view_range range) const
{
    return texture_view::create(this, range, format, type);
}

texture_layout texture::get_layout() const
{
    return m_layout;
}

bool texture::is_swapchain_owned() const
{
    return m_isSwapchain;
}

} // gfx