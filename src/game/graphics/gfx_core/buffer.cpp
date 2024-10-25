#include "buffer.h"

namespace gfx
{

buffer::buffer(memory_info allocation, buffer_usage usage, void* pImpl) :
    resource(allocation),
    m_impl(pImpl),
    m_usage(usage)
{ }

buffer_usage buffer::get_usage() const
{
    return m_usage;
}

} // gfx