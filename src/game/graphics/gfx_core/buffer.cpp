#include "buffer.h"

namespace gfx
{

buffer::buffer(memory_info allocation, buffer_usage usage, void* pImpl) :
    m_allocation(allocation),
    m_impl(pImpl),
    m_usage(usage)
{ }

memory_info& buffer::get_allocation()
{
    return m_allocation;
}

buffer_usage buffer::get_usage() const
{
    return m_usage;
}

void* buffer::get_impl_ptr()
{
    return m_impl;
}

} // gfx