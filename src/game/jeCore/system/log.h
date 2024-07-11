#pragma once

namespace sys
{
namespace log
{

enum channel
{
    none = 1 << 0,
    system = 1 << 1,
    graphics = 1 << 2,

    count = 3,
};

enum class level
{
    none,
    profile,
    info,
    warn,
    error,
    fatal,

    count,
    disable,
};

inline constexpr const char* channel_to_string(channel chnl)
{
    switch( chnl )
    {
    case channel::system:
        return "system";
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

int initialise();
void shutdown();

void message(std::string message, channel chnl = channel::none, level lvl = level::none);
void profile(std::string message, channel chnl = channel::none);
void info(std::string message, channel chnl = channel::none);
void warn(std::string message, channel chnl = channel::none);
void error(std::string message, channel chnl = channel::none);
void fatal(std::string message, channel chnl = channel::none);

} // log
} // sys