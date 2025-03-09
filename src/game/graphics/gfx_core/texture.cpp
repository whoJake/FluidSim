#include "texture.h"

namespace gfx
{

void texture::initialise(memory_info allocation,
                         texture_info info,
                         void* pImpl,
                         void* pImplView,
                         bool isSwapchainImage)
{
    
    resource::initialise(allocation);
    texture_info::initialise(info);
    m_pImpl = pImpl;
    m_pImplView = pImplView;
    m_isSwapchain = isSwapchainImage;
}

void texture::set_resource_view_type(resource_view_type type, void* pImplView)
{
    m_memoryInfo.viewType = u32_cast(type);
    m_pImplView = pImplView;
}

bool texture::is_swapchain_image() const
{
    return m_isSwapchain;
}

void texture_info::initialise(const texture_info& other)
{
    initialise(other.m_format, other.m_usage, other.m_width, other.m_height, other.m_depthOrLayers);
}

void texture_info::initialise(cdt::image_format format, texture_usage_flags usage, u32 width, u32 height, u32 depthOrLayers)
{
    m_format = format;
    m_width = width;
    m_height = height;
    m_depthOrLayers = depthOrLayers;
    m_usage = usage;
}

cdt::image_format texture_info::get_format() const
{
    return m_format;
}

u32 texture_info::get_width() const
{
    return m_width;
}

u32 texture_info::get_height() const
{
    return m_height;
}

u32 texture_info::get_depth() const
{
    return m_depthOrLayers;
}

texture_usage_flags texture_info::get_usage() const
{
    return m_usage;
}

u64 texture_info::get_size() const
{
    return u64_cast(m_width)
        * m_height
        * m_depthOrLayers
        * cdt::get_bits_per_pixel(m_format);
}

} // gfx