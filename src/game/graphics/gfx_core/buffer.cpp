#include "buffer.h"
#include "driver.h"

namespace gfx
{

buffer buffer::create(const memory_info& memory_info)
{
    buffer retval{ };
    driver::create_buffer(&retval, memory_info);
    return retval;
}

void buffer::destroy(buffer* buffer)
{
    driver::destroy_buffer(buffer);
}

buffer_view buffer::create_view(format format, resource_view_type type, buffer_view_range range) const
{
    return buffer_view::create(this, range, format, type);
}

} // gfx