#pragma once

#include "LogTarget.h"
#include <vector>

namespace jclog
{

class Log
{
public:
    Log():
        m_targets()
    { };
    ~Log()
    { 
        for( LogTarget* target : m_targets )
        {
            delete target;
        }
    }
    Log(Log&&) = delete;
    Log(const Log& other) = delete;

    void register_target(LogTarget* target)
    {
        m_targets.push_back(target);
    }

    template<typename... A>
    void none(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::NONE, str.c_str());
    }

    template<typename... A>
    void trace(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::TRACE, str.c_str());
    }

    template<typename... A>
    void track(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::TRACK, str.c_str());
    }

    template<typename... A>
    void debug(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::DEBUG, str.c_str());
    }

    template<typename... A>
    void profile(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::PROFILE, str.c_str());
    }

    template<typename... A>
    void info(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::INFO, str.c_str());
    }

    template<typename... A>
    void warn(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::WARN, str.c_str());
    }

    template<typename... A>
    void error(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::ERROR, str.c_str());
    }

    template<typename... A>
    void event(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::EVENT, str.c_str());
    }

    template<typename... A>
    void exception(const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::EXCEPTION, str.c_str());
    }

    template<typename... A>
    void exception(std::exception exception, const char* fmt, A&&... args) const
    {
        std::string str = std::vformat(fmt, std::make_format_args(args...));
        log(Level::NONE, exception, str.c_str());
    }

    void log(Level level, const char* logstr) const
    {
        for( const auto target : m_targets )
        {
            target->log(level, logstr);
        }
    }

    void log(Level level,std::exception ex, const char* logstr) const
    {
        for( const auto target : m_targets )
        {
            target->log(level, ex, logstr);
        }
    }

private:
    std::vector<LogTarget*> m_targets;
};

} // jclog