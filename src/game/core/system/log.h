#pragma once

#include <format>

namespace sys
{
namespace log
{

enum channel
{
    none = 1 << 0,
    system = 1 << 1,
    vulkan = 1 << 2,
    graphics = 1 << 3,

    count = 3,
};

enum class level
{
    none,
    profile,
    debug,
    info,
    warn,
    error,
    fatal,

    count,
    disable,
};

int initialise();
void shutdown();

void message(std::string message, channel chnl = channel::none, level lvl = level::none);
void profile(std::string message, channel chnl = channel::none);
void info(std::string message, channel chnl = channel::none);
void warn(std::string message, channel chnl = channel::none);
void error(std::string message, channel chnl = channel::none);
void fatal(std::string message, channel chnl = channel::none);

void force_flush();

template<typename... A>
void messagef(channel chnl, level lvl, std::string format, A&&... args)
{
    message(std::vformat(format, std::make_format_args(args...)), chnl, lvl);
}

#define CHANNEL_LOG_MSG(chnl, fmt, ...) ::sys::log::messagef(chnl, ::sys::log::level::none, fmt, __VA_ARGS__)
#define CHANNEL_LOG_PROFILE(chnl, fmt, ...) ::sys::log::messagef(chnl, ::sys::log::level::profile, fmt, __VA_ARGS__)
#define CHANNEL_LOG_DEBUG(chnl, fmt, ...) ::sys::log::messagef(chnl, ::sys::log::level::debug, fmt, __VA_ARGS__)
#define CHANNEL_LOG_INFO(chnl, fmt, ...) ::sys::log::messagef(chnl, ::sys::log::level::info, fmt, __VA_ARGS__)
#define CHANNEL_LOG_WARN(chnl, fmt, ...) ::sys::log::messagef(chnl, ::sys::log::level::warn, fmt, __VA_ARGS__)
#define CHANNEL_LOG_ERROR(chnl, fmt, ...) ::sys::log::messagef(chnl, ::sys::log::level::error, fmt, __VA_ARGS__)
#define CHANNEL_LOG_FATAL(chnl, fmt, ...) ::sys::log::messagef(chnl, ::sys::log::level::fatal, fmt, __VA_ARGS__)

#define SYSLOG_MSG(fmt, ...) CHANNEL_LOG_MSG(::sys::log::channel::none, fmt, __VA_ARGS__)
#define SYSLOG_PROFILE(fmt, ...) CHANNEL_LOG_PROFILE(::sys::log::channel::none, fmt, __VA_ARGS__)
#define SYSLOG_DEBUG(fmt, ...) CHANNEL_LOG_DEBUG(::sys::log::channel::none, fmt, __VA_ARGS__)
#define SYSLOG_INFO(fmt, ...) CHANNEL_LOG_INFO(::sys::log::channel::none, fmt, __VA_ARGS__)
#define SYSLOG_WARN(fmt, ...) CHANNEL_LOG_WARN(::sys::log::channel::none, fmt, __VA_ARGS__)
#define SYSLOG_ERROR(fmt, ...) CHANNEL_LOG_ERROR(::sys::log::channel::none, fmt, __VA_ARGS__)
#define SYSLOG_FATAL(fmt, ...) CHANNEL_LOG_FATAL(::sys::log::channel::none, fmt, __VA_ARGS__)

#define SYSLOG_FORCEFLUSH() ::sys::log::force_flush()

enum class color
{
    none,
    black,
    dark_red,
    dark_green,
    dark_yellow,
    dark_blue,
    dark_magenta,
    dark_cyan,
    light_gray,
    dark_gray,
    red,
    green,
    orange,
    blue,
    magenta,
    cyan,
    white,

    count,
};

inline constexpr const char* color_to_code(color c)
{
    switch( c )
    {
    case color::black:
        return "\033[30m";
    case color::dark_red:
        return "\033[31m";
    case color::dark_green:
        return "\033[32m";
    case color::dark_yellow:
        return "\033[33m";
    case color::dark_blue:
        return "\033[34m";
    case color::dark_magenta:
        return "\033[35m";
    case color::dark_cyan:
        return "\033[36m";
    case color::light_gray:
        return "\033[37m";
    case color::dark_gray:
        return "\033[90m";
    case color::red:
        return "\033[91m";
    case color::green:
        return "\033[92m";
    case color::orange:
        return "\033[93m";
    case color::blue:
        return "\033[94m";
    case color::magenta:
        return "\033[95m";
    case color::cyan:
        return "\033[96m";
    case color::white:
        return "\033[97m";
    case color::none:
    default:
        return "\033[0m";
    }
}

inline constexpr const char* channel_to_string(channel chnl)
{
    switch( chnl )
    {
    case channel::system:
        return "system";
    case channel::vulkan:
        return "vulkan";
    case channel::graphics:
        return "graphics";
    case channel::none:
    default:
        return "misc";
    }
}

inline level parse_level(const char* string)
{
    if( !string )
        return level::none;
    if( !strcmp(string, "") )
        return level::none;
    if( !strcmp(string, "profile") )
        return level::profile;
    if( !strcmp(string, "debug") )
        return level::debug;
    if( !strcmp(string, "info") )
        return level::info;
    if( !strcmp(string, "warn") )
        return level::warn;
    if( !strcmp(string, "error") )
        return level::error;
    if( !strcmp(string, "fatal") )
        return level::fatal;

    return level::none;
}

inline constexpr const char* level_to_string(level lvl)
{
    switch( lvl )
    {
    case level::none:
        return "";
    case level::profile:
        return "prof";
    case level::debug:
        return "debug";
    case level::info:
        return "info";
    case level::warn:
        return "warn";
    case level::error:
        return "error";
    case level::fatal:
        return "fatal";
    default:
        return " ??? ";
    }
}

} // log
} // sys