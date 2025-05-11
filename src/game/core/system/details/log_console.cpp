#include "log_console.h"

#include <iostream>
#include "helpers/string_manip.h"

namespace sys
{
namespace log
{
namespace details
{

console_target::console_target() :
    m_levelToColor(u8_cast(level::count))
{
    m_levelToColor[u8_cast(level::none)] = color::none;
    m_levelToColor[u8_cast(level::verbose)] = color::dark_green;
    m_levelToColor[u8_cast(level::profile)] = color::green;
    m_levelToColor[u8_cast(level::debug)] = color::cyan;
    m_levelToColor[u8_cast(level::info)] = color::blue;
    m_levelToColor[u8_cast(level::warn)] = color::dark_yellow;
    m_levelToColor[u8_cast(level::error)] = color::red;
    m_levelToColor[u8_cast(level::fatal)] = color::dark_red;
}

console_target::~console_target()
{ }

void console_target::output(message* msg)
{
    for( std::string line : split_string(msg->text, "\n") )
    {
        std::cout << std::format("[tid:{:0>8}] [{}{: <5}{}] [{: ^10}] {}\n",
            *(u32*)&msg->threadid,
            color_to_code(m_levelToColor[u8_cast(msg->lvl)]),
            level_to_string(msg->lvl),
            color_to_code(color::none),
            channel_to_string(msg->chnl),
            line);
    }
}

} // details
} // log
} // sys