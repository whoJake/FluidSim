#include "debugger.h"

namespace gfx
{

bool debugger::is_enabled() const
{
    return m_enabled;
}

void debugger::attach_callback(callback func)
{
    m_callbacks.push_back(func);
}

void debugger::send_event(severity level, const char* message)
{
    for( const callback& cb : m_callbacks )
    {
        cb(level, message);
    }
}

void debugger::set_impl_ptr(void* data)
{
    m_impl = data;
}

void* debugger::get_impl_ptr() const
{
    return m_impl;
}

} // gfx