#pragma once

#include "../log.h"
#include "log_message.h"

namespace sys
{
namespace log
{
namespace details
{

class target
{
public:
    virtual void output(message* msg) = 0;
};

class logger
{
public:
    logger();
    ~logger();

    logger(logger&&) noexcept;
    logger(const logger&) = delete;

    logger& operator=(logger&&) noexcept;
    logger& operator=(const logger&) = delete;

    void output(message* msg);

    void register_target(target* ptarget);
private:
    std::vector<target*> m_targets;
};

class log_manager
{
public:
    log_manager(level verbosity);

    void add_message(message&& msg);

    virtual void flush() = 0;
protected:
    virtual void assign_message(message&& msg) = 0;
private:
    level m_verbosity;
};

} // details
} // log
} // sys