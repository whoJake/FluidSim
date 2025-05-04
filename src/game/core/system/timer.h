#pragma once

#include <sstream>
#include <iostream>
#include <chrono>
#include <format>
#include "assert.h"

namespace sys
{

using no_fidelity = uint8_t;    
using nanoseconds = std::chrono::nanoseconds;
using microseconds = std::chrono::microseconds;
using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;
using minutes = std::chrono::minutes;
    
using moment = std::chrono::steady_clock::time_point;

static constexpr long long nanoseconds_in_microsecond = static_cast<long long>(1e3);
static constexpr long long microseconds_in_millisecond = static_cast<long long>(1e3);
static constexpr long long milliseconds_in_second = static_cast<long long>(1e3);
static constexpr long long seconds_in_minute = static_cast<long long>(60);
static constexpr long long minutes_in_hour = static_cast<long long>(60);

[[nodiscard]] static moment now()
{
    return std::chrono::high_resolution_clock::now();
}

template<typename fidelity>   
class timer
{
public:
    // Goodness me this is cursed
    using subfidelity = std::conditional_t<(
        std::is_same_v<fidelity, nanoseconds>), no_fidelity, std::conditional_t<(
        std::is_same_v<fidelity, microseconds>), nanoseconds, std::conditional_t<(
        std::is_same_v<fidelity, milliseconds>), microseconds, std::conditional_t<(
        std::is_same_v<fidelity, seconds>), milliseconds, std::conditional_t<(
        std::is_same_v<fidelity, minutes>), seconds, no_fidelity>>>>>;
    
    static_assert(
           std::is_same_v<fidelity, nanoseconds>
        || std::is_same_v<fidelity, microseconds>
        || std::is_same_v<fidelity, milliseconds>
        || std::is_same_v<fidelity, seconds>
        || std::is_same_v<fidelity, minutes>
        , "Timer template must be instantiated with a std::chrono::duration type.");

    timer(sys::log::channel channel, const char* format) :
        m_start(now()),
        m_format(format),
        m_channel(channel)
    { }
    
    timer(const char* format) :
        m_start(now()),
        m_format(format),
        m_channel(sys::log::channel::none)
    { }
    
    timer() :
        timer("Timer finished: {}")
    { }

    ~timer()
    {
        output_timer();
    }

    [[nodiscard]] static constexpr const char* fidelity_suffix()
    {
        if constexpr (std::is_same_v<fidelity, nanoseconds>)
        {
            return "ns";
        }
        else if constexpr (std::is_same_v<fidelity, microseconds>)
        {
            return "us";
        }
        else if constexpr (std::is_same_v<fidelity, milliseconds>)
        {
            return "ms";
        }
        else if constexpr (std::is_same_v<fidelity, seconds>)
        {
            return "s";
        }
        else if constexpr (std::is_same_v<fidelity, minutes>)
        {
            return "m";
        }
        else
        {
            static_assert(true, "Unreachable");
            return "";
        }
    }
private:
    [[nodiscard]] constexpr long long get_subfidelity_count(moment end) const
     {
         auto diff = end - m_start;
         long long modulo = std::numeric_limits<long long>::max();

         if constexpr (std::is_same_v<subfidelity, no_fidelity>)
         {
             return 0;
         }
         else if constexpr (std::is_same_v<subfidelity, minutes>)
         {
             modulo = minutes_in_hour;
         }
         else if constexpr (std::is_same_v<subfidelity, seconds>)
         {
             modulo = seconds_in_minute;
         }
         else if constexpr (std::is_same_v<subfidelity, milliseconds>)
         {
             modulo = milliseconds_in_second;
         }
         else if constexpr (std::is_same_v<subfidelity, microseconds>)
         {
             modulo = microseconds_in_millisecond;
         }
         else if constexpr (std::is_same_v<subfidelity, nanoseconds>)
         {
             modulo = nanoseconds_in_microsecond;
         }
         else
         {
             static_assert(true);
         }
        
         return std::chrono::floor<subfidelity>(diff).count() % modulo;
     }

     [[nodiscard]] std::string get_fidelity_string() const
     {
        moment end = now();
        auto diff = end - m_start;
        
        long long fidelityCount = std::chrono::floor<fidelity>(diff).count();
        long long subFidelityCount = get_subfidelity_count(end);

         return std::format("{}.{:0>3}", fidelityCount, subFidelityCount);
     }
     
    inline void output_timer() const
    {
        std::string time = std::format("{}{}", get_fidelity_string(), fidelity_suffix());
        SYSMSG_PROFILE(m_format, time);
    }
private:
    moment m_start;
    const char* m_format;
    ::sys::log::channel m_channel;
};

} // sys