#include "log.h"
#include "details/log_details.h"
#include "details/log_console.h"
#include "details/basic_log.h"
#include "details/log_mt.h"

namespace sys
{
namespace log
{

PARAM(no_log);
PARAM(log_verbosity);
PARAM(disable_file_logging);

details::log_manager* s_log = nullptr;

int log::initialise()
{
    if( Param_no_log.get() )
    {
        s_log = new details::basic_log(level::disable);
        return 1;
    }

    details::logger logger;
    logger.register_target(new details::console_target());
    if( !Param_disable_file_logging.get() )
    {
        // register files
    }

    level verbosity = parse_level(Param_log_verbosity.value());
    s_log = new details::log_mt(std::move(logger), verbosity, 8192, 1);
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

} // log
} // sys