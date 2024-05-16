#include "ConsoleTarget.h"

#include <iostream>
#include <format>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace jclog
{

static void pad_stringstream(std::stringstream* ss, uint32_t padWidth)
{
    for( uint32_t i = 0; i < padWidth; i++ )
        (*ss) << " ";
}

ConsoleTarget::ConsoleTarget(bool hasTimestamp, const char* timestampFormat):
    m_hasTimestamp(hasTimestamp),
    m_timestampFormat(timestampFormat)
{ 
    m_padWidth += 8; // [{5 spaces}]{1 space}
    if( m_hasTimestamp )
        m_padWidth += (uint32_t)strlen(timestampFormat) + 3; // []{1 space}
}

ConsoleTarget::~ConsoleTarget()
{ }

void ConsoleTarget::log(Level level, const char* logstr) const
{
    if( m_hasTimestamp )
    {
        time_t time = std::time(nullptr);

    #pragma warning( push )
    #pragma warning( disable : 4996 )
        std::tm timeStruct = *std::localtime(&time);
    #pragma warning( pop )

        std::cout << "[" << std::put_time(&timeStruct, m_timestampFormat) << "] ";
    }

    std::cout << "[";

    switch( level )
    {
    case Level::TRACE:
        std::cout << LOG_COLOR_GRAY;
        break;
    case Level::TRACK:
        std::cout << LOG_COLOR_MAGENTA;
        break;
    case Level::DEBUG:
        std::cout << LOG_COLOR_CYAN;
        break;
    case Level::PROFILE:
        std::cout << LOG_COLOR_BLUE;
        break;
    case Level::INFO:
        std::cout << LOG_COLOR_YELLOW;
        break;
    case Level::WARN:
        std::cout << LOG_COLOR_BOLD << LOG_COLOR_YELLOW;
        break;
    case Level::ERROR:
        std::cout << LOG_COLOR_RED;
        break;
    case Level::EVENT:
        std::cout << LOG_COLOR_GREEN;
        break;
    case Level::EXCEPTION:
        std::cout << LOG_COLOR_BOLD << LOG_COLOR_RED;
        break;
    default:
        std::cout << LOG_COLOR_WHITE;
        break;
    }

    std::cout << level_prefix(level) << LOG_COLOR_RESET << "] " << logstr << std::endl;
}

void ConsoleTarget::log(Level level, std::exception exception, const char* logstr) const
{
    log(level, logstr);

    // Exception formatting?
    std::string exStr = std::format("Exception:\n{}", exception.what());
    this->log(Level::NONE, exStr.c_str());
}

} // jclog