#include "allocator.h"

namespace gfx
{

allocator::allocator(void* pImpl) :
    m_impl(pImpl)
{ }

void allocator::set_impl_ptr(void* pImpl)
{
    m_impl = pImpl;
}

void* allocator::get_impl_ptr()
{
    return m_impl;
}

const void* allocator::get_impl_ptr() const
{
    return m_impl;
}

} // gfx