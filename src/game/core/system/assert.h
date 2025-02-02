#pragma once

namespace sys
{

enum severity
{
    SEVERITY_ALL,

    SEVERITY_VERBOSE,
    SEVERITY_PROFILE,
    SEVERITY_DEBUG,
    SEVERITY_INFO,
    SEVERITY_WARN,
    SEVERITY_ERROR,
    SEVERITY_ASSERT,
    SEVERITY_FATAL,

    SEVERITY_COUNT,
    SEVERITY_DISABLE,
};

#define SYSDECLARE_CHANNEL(name) inline static ::sys::channel c_##name(#name)
#define SYSDECLARE_SUBCHANNEL(parent, name) inline static ::sys::channel c_##parent_##name(#name, &c_##parent)

class channel
{
public:
    static channel* get_global_channel();

    channel(const char* name);
    channel(const char* name, channel* parent);
    ~channel() = default;

    void set_severity(severity value);
    severity get_severity() const;

    const char* get_name() const;
    channel* get_parent() const;

    bool is_enabled(severity level) const;
private:
    struct nullptr_parent_t
    { };
    channel(const char* name, nullptr_parent_t token);
private:
    const char* m_name;
    channel* m_parent;
    severity m_severity;
};

using quit_handler = std::function<void(const channel&, std::string)>;
using quit_message_generator = std::function<std::string(const channel&)>;

[[noreturn]]
void quit(const channel& channel);

[[noreturn]]
void fast_quit();

void set_quit_handler(quit_handler func);
void set_quit_message_generator(quit_message_generator func);

#define SYSASSERT(cond, res) if(!(cond))[[unlikely]]do{res;}while(0)

#define SYSMSG_VERBOSE(fmt, ...) ::sys::message_handler::send_verbose(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)
#define SYSMSG_PROFILE(fmt, ...) ::sys::message_handler::send_profile(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)
#define SYSMSG_DEBUG(fmt, ...) ::sys::message_handler::send_debug(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)
#define SYSMSG_INFO(fmt, ...) ::sys::message_handler::send_info(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)
#define SYSMSG_WARN(fmt, ...) ::sys::message_handler::send_warn(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)
#define SYSMSG_ERROR(fmt, ...) ::sys::message_handler::send_error(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)
#define SYSMSG_ASSERT(fmt, ...) ::sys::message_handler::send_assert(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)
#define SYSMSG_FATAL(fmt, ...) ::sys::message_handler::send_fatal(*::sys::channel::get_global_channel(), fmt, __VA_ARGS__)

#define SYSMSG_CHANNEL_VERBOSE(channel, fmt, ...) ::sys::message_handler::send_verbose(c_##channel, fmt, __VA_ARGS__)
#define SYSMSG_CHANNEL_PROFILE(channel, fmt, ...) ::sys::message_handler::send_profile(c_##channel, fmt, __VA_ARGS__)
#define SYSMSG_CHANNEL_DEBUG(channel, fmt, ...) ::sys::message_handler::send_debug(c_##channel, fmt, __VA_ARGS__)
#define SYSMSG_CHANNEL_INFO(channel, fmt, ...) ::sys::message_handler::send_info(c_##channel, fmt, __VA_ARGS__)
#define SYSMSG_CHANNEL_WARN(channel, fmt, ...) ::sys::message_handler::send_warn(c_##channel, fmt, __VA_ARGS__)
#define SYSMSG_CHANNEL_ERROR(channel, fmt, ...) ::sys::message_handler::send_error(c_##channel, fmt, __VA_ARGS__)
#define SYSMSG_CHANNEL_ASSERT(channel, fmt, ...) ::sys::message_handler::send_assert(c_##channel, fmt, __VA_ARGS__)
#define SYSMSG_CHANNEL_FATAL(channel, fmt, ...) ::sys::message_handler::send_fatal(c_##channel, fmt, __VA_ARGS__)

using message_callback = std::function<void(const channel&, severity, std::string formatted_message)>;

class message_handler
{
private:
    template<typename... Args>
    static std::string format_message(const char* fmt, Args&&... args)
    {
        return std::vformat(fmt, std::make_format_args(args...));
    }
public:
    static bool send_message(const channel& channel, severity level, std::string formatted_message);

    template<typename... Args>
    static bool send_verbose(const channel& channel, const char* fmt, Args&&... args)
    {
        if( !channel.is_enabled(sys::SEVERITY_VERBOSE) )
            return false;

        return send_message(channel, sys::SEVERITY_VERBOSE, format_message(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static bool send_profile(const channel& channel, const char* fmt, Args&&... args)
    {
        if( !channel.is_enabled(sys::SEVERITY_PROFILE) )
            return false;
        
        return send_message(channel, sys::SEVERITY_PROFILE, format_message(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static bool send_debug(const channel& channel, const char* fmt, Args&&... args)
    {
        if( !channel.is_enabled(sys::SEVERITY_DEBUG) )
            return false;

        return send_message(channel, sys::SEVERITY_DEBUG, format_message(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static bool send_info(const channel& channel, const char* fmt, Args&&... args)
    {
        if( !channel.is_enabled(sys::SEVERITY_INFO) )
            return false;

        return send_message(channel, sys::SEVERITY_INFO, format_message(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static bool send_warn(const channel& channel, const char* fmt, Args&&... args)
    {
        if( !channel.is_enabled(sys::SEVERITY_WARN) )
            return false;

        return send_message(channel, sys::SEVERITY_WARN, format_message(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static bool send_error(const channel& channel, const char* fmt, Args&&... args)
    {
        if( !channel.is_enabled(sys::SEVERITY_ERROR) )
            return false;

        return send_message(channel, sys::SEVERITY_ERROR, format_message(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static bool send_assert(const channel& channel, const char* fmt, Args&&... args)
    {
        if( !channel.is_enabled(sys::SEVERITY_ASSERT) )
            return false;

        return send_message(channel, sys::SEVERITY_ASSERT, format_message(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    static bool send_fatal(const channel& channel, const char* fmt, Args&&... args)
    {
        send_message(channel, sys::SEVERITY_FATAL, format_message(fmt, std::forward<Args>(args)...));
        quit(channel);
    }

    static void set_message_callback(message_callback cb);
    static void set_message_callback_hook(message_callback cb);
private:
    static message_callback sm_callback;
    static message_callback sm_hook;
};

} // sys