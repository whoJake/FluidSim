#include "resource_view.h"
#include "driver.h"

namespace gfx
{

buffer_view_range full_buffer_view = { u64_max, u64_max };
texture_view_range full_texture_view = { u16_max, u16_max, u16_max, u16_max };

format resource_view::get_format() const
{
    return m_format;
}

resource_view_type resource_view::get_type() const
{
    return m_type;
}

const buffer* buffer_view::get_resource() const
{
    return static_cast<const buffer*>(m_pResource);
}

buffer_view buffer_view::create(const buffer* buffer, buffer_view_range range, format format, resource_view_type type)
{
    buffer_view retval{ };
    driver::create_buffer_view(&retval, buffer, range, format, type);
    return retval;
}

void buffer_view::destroy(buffer_view* buffer_view)
{
    driver::destroy_buffer_view(buffer_view);
}

const texture* texture_view::get_resource() const
{
    return static_cast<const texture*>(m_pResource);
}

texture_view texture_view::create(const texture* texture, texture_view_range range, format format, resource_view_type type)
{
    texture_view retval{ };
    driver::create_texture_view(&retval, texture, range, format, type);
    return retval;
}

void texture_view::destroy(texture_view* texture_view)
{
    driver::destroy_texture_view(texture_view);
}

} // gfx