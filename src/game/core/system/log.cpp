#include "log.h"
#include "details/log_details.h"
#include "details/log_console.h"
#include "details/basic_log.h"
#include "details/log_mt.h"

namespace sys
{
namespace log
{

details::log_manager* s_log = nullptr;

int log::initialise(details::log_manager* log)
{
    s_log = log;
    return 1;
}

void log::shutdown()
{
    delete s_log;
}

void log::message(std::string message, channel chnl, level lvl)
{
    s_log->add_message({ message, chnl, lvl });
}

void log::profile(std::string message, channel chnl)
{
    log::message(message, chnl, level::profile);
}

void log::info(std::string message, channel chnl)
{
    log::message(message, chnl, level::info);
}

void log::warn(std::string message, channel chnl)
{
    log::message(message, chnl, level::warn);
}

void log::error(std::string message, channel chnl)
{
    log::message(message, chnl, level::error);
}

void log::fatal(std::string message, channel chnl)
{
    log::message(message, chnl, level::fatal);
}

void log::force_flush()
{
    s_log->flush();
}

} // log
} // sys