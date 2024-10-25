#include "texture.h"

namespace gfx
{

texture::texture(memory_info allocation,
                 texture_info info,
                 void* pImpl,
                 void* pImplView) :
    resource(allocation),
    texture_info(info),
    m_impl(pImpl),
    m_implView(pImplView)
{ }

void texture::set_resource_view_type(resource_view_type type, void* pImplView)
{
    m_memoryInfo.viewType = u32_cast(type);
    m_implView = pImplView;
}

texture_info::texture_info(cdt::image_format format, texture_usage_flags usage, u32 width, u32 height, u32 depthOrLayers) :
    m_format(format),
    m_width(width),
    m_height(height),
    m_depthOrLayers(depthOrLayers),
    m_usage(usage)
{ }

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