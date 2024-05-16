#pragma once

#include "LogTarget.h"
#include <fstream>

namespace jclog
{

class FileTarget : public LogTarget
{
public:
    FileTarget(const char* logDirectory);
    ~FileTarget();
    FileTarget(FileTarget&&) = delete;
    FileTarget(const FileTarget&) = delete;

protected:
    void log(Level level, const char* logstr) const;
    void log(Level level, std::exception exception, const char* logstr) const;

private:
    mutable std::fstream m_logFile;
};

}