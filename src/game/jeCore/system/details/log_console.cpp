#include "log_console.h"

#include <iostream>
#include "helpers/string_manip.h"

namespace sys
{
namespace log
{
namespace details
{

console_target::console_target()
{ }

console_target::~console_target()
{ }

void console_target::output(message* msg)
{
    for( std::string line : split_string(msg->text, "\n") )
    {
        std::cout << std::format("[tid:{:0>8}] [{: <5}] [{: ^10}] {}\n", *(u32*)&msg->threadid, level_to_string(msg->lvl), channel_to_string(msg->chnl), line);
    }
}

} // details
} // log
} // sys