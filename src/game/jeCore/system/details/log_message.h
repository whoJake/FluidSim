#pragma once

#include "../log.h"
#include <thread>
#include <chrono>

namespace sys
{
namespace log
{
namespace details
{

struct message
{
    // This can be reduced to a fixed string to save 8 bytes
    std::string text;
    channel chnl;
    level lvl;
    std::thread::id threadid = std::this_thread::get_id();
    std::chrono::steady_clock::time_point timestamp = std::chrono::high_resolution_clock::now();
};

} // details
} // log
} // sys