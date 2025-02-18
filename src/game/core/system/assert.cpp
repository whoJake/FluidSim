#include "assert.h"

#ifndef NOMINMAX
    #define NOMINMAX
#endif // NOMINMAX

// Have to include Windows.h before including debugapi for reasons of
// I have no idea its just required.
// https://stackoverflow.com/questions/4845198/fatal-error-no-target-architecture-in-visual-studio
#include "Windows.h"
#include "debugapi.h"

#include <iostream>
#include <sstream>

namespace sys
{

channel* channel::get_global_channel()
{
    static channel all("all", { });
    return &all;
}

channel::channel(const char* name) :
    m_name(name),
    m_severity(SEVERITY_ALL)
{
    m_parent = get_global_channel();
}

channel::channel(const char* name, channel* parent) :
    m_name(name),
    m_parent(parent),
    m_severity(SEVERITY_ALL)
{ }

channel::channel(const char* name, nullptr_parent_t) :
    m_name(name),
    m_parent(nullptr),
    m_severity(SEVERITY_ALL)
{ }

void channel::set_severity(severity value)
{
    m_severity = value;
}

severity channel::get_severity() const
{
    return m_severity;
}

const char* channel::get_name() const
{
    return m_name;
}

channel* channel::get_parent() const
{
    return m_parent;
}

bool channel::is_enabled(severity level) const
{
    if( m_parent && !m_parent->is_enabled(level) )
        return false;

    return u32_cast(level) >= u32_cast(m_severity);
}

static void default_quit_handler(const channel& channel, std::string quit_message)
{
    assert(0);
}

static std::string default_message_generator(const channel& channel)
{
    return std::string("Quit on channel ") + channel.get_name();
}

static quit_handler g_quitHandler = default_quit_handler;
static quit_message_generator g_quitMessageGenerator = default_message_generator;

void quit(const channel& channel)
{
    g_quitHandler(channel, g_quitMessageGenerator(channel));
}

void fast_quit()
{
    assert(0);
}

void set_quit_handler(quit_handler func)
{
    g_quitHandler = func;
}

void set_quit_message_generator(quit_message_generator func)
{
    g_quitMessageGenerator = func;
}

bool is_debugger_present()
{
    return IsDebuggerPresent();
}

static void default_message_callback(const channel& channel, severity level, std::string formatted_message)
{
    std::stringstream ss;
    ss << "[" << channel.get_name() << "] " << formatted_message;
    std::cout << ss.str() << std::endl;
}

static void default_message_hook(const channel& channel, severity level, std::string formatted_message)
{ }

message_callback message_handler::sm_callback = default_message_callback;
message_callback message_handler::sm_hook = default_message_hook;

bool message_handler::send_message(const channel& channel, severity level, std::string formatted_message)
{
    sm_hook(channel, level, formatted_message);
    sm_callback(channel, level, formatted_message);
    return true;
}

void message_handler::set_message_callback(message_callback cb)
{
    sm_callback = cb;
}

void message_handler::set_message_callback_hook(message_callback cb)
{
    sm_hook = cb;
}

} // sys