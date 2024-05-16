#include "FileTarget.h"
#include <fstream>
#include <sstream>
#include <chrono>

namespace jclog
{

FileTarget::FileTarget(const char* logDirectory):
    m_logFile()
{
    // Delete file if it already exists
    m_logFile.open(logDirectory, std::ios::in);
    if( m_logFile.good() )
        std::remove(logDirectory);

    m_logFile.close();

    // Open file for logging
    m_logFile.open(logDirectory, std::ios::out);

    if( m_logFile.bad() )
    {
        std::stringstream ss;
        ss << "Failed to create log file '" << logDirectory << "'";
        throw new std::exception(ss.str().c_str());
    }
}

FileTarget::~FileTarget()
{
    m_logFile.close();
}

void FileTarget::log(Level level, const char* logstr) const
{
    auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    m_logFile << epoch.count() << "," << logstr << "\n";
}

void FileTarget::log(Level level, std::exception exception, const char* logstr) const
{
    std::string combine = std::format("{},{}", logstr, exception.what());
    log(level, combine.c_str());
}

} // jclog