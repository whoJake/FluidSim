#pragma once
#include "LogTarget.h"

namespace jclog
{

class ConsoleTarget: public LogTarget
{
public:
    ConsoleTarget(bool hasTimestamp = true, const char* timestampFormat = "%H:%M:%S");
    //ConsoleLog(bool hasTimestamp = true, const char* timestampFormat = "%Y-%m-%d %H:%M:%S");
    ~ConsoleTarget() override;
    ConsoleTarget(ConsoleTarget&&) = delete;
    ConsoleTarget(const ConsoleTarget&) = delete;

protected:
    void log(Level level, const char* logstr) const;
    void log(Level level, std::exception exception, const char* logstr) const;

private:
    bool m_hasTimestamp;
    const char* m_timestampFormat;
    uint32_t m_padWidth = 0;
};

} // jclog