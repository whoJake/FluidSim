#pragma once

#include "log_details.h"

namespace sys
{
namespace log
{
namespace details
{

class basic_log : public log_manager
{
public:
    basic_log(level verbosity = level::none);
    basic_log(logger&& log, level verbosity = level::none);
    ~basic_log() = default;

    void assign_message(message&& msg) override;

    inline void flush() override { };
private:
    logger m_logger;
};

} // details
} // log
} // sys