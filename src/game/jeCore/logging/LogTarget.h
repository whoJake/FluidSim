#pragma once

#include <exception>
#include <cstdarg>
#include <format>
#include <string>

namespace jclog
{

#define LOG_COLOR_RESET "\033[0m"
#define LOG_COLOR_BOLD "\033[1m"
#define LOG_COLOR_GRAY "\033\30"
#define LOG_COLOR_RED "\033[31m"
#define LOG_COLOR_GREEN "\033[32m"
#define LOG_COLOR_YELLOW "\033[33m"
#define LOG_COLOR_BLUE "\033[34m"
#define LOG_COLOR_MAGENTA "\033[35m"
#define LOG_COLOR_CYAN "\033[36m"
#define LOG_COLOR_WHITE "\033[37m"

enum class Level
{
    NONE,
    TRACE,
    TRACK,
    DEBUG,
    PROFILE,
    INFO,
    WARN,
    ERROR,
    EVENT,
    EXCEPTION
};

static constexpr const char* level_prefix(Level level)
{
    switch( level )
    {
    case Level::NONE:
        return "     ";
    case Level::TRACE:
        return "trace";
    case Level::TRACK:
        return "track";
    case Level::DEBUG:
        return "debug";
    case Level::PROFILE:
        return "prof ";
    case Level::INFO:
        return "info ";
    case Level::WARN:
        return "warn ";
    case Level::ERROR:
        return "error";
    case Level::EVENT:
        return "event";
    case Level::EXCEPTION:
        return "ex   ";
    default:
        return "     ";
    }
}

class LogTarget
{
public:
    virtual ~LogTarget() { };

    virtual void log(Level level, const char* logstr) const = 0;
    virtual void log(Level level, std::exception exception, const char* logstr) const = 0;
protected:
    LogTarget() { };
};

} // jclog