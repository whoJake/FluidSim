#include "basic_log.h"

namespace sys
{
namespace log
{
namespace details
{

basic_log::basic_log(level verbosity) :
    basic_log({ }, verbosity)
{ }

basic_log::basic_log(logger&& log, level verbosity) :
    log_manager(verbosity),
    m_logger(std::move(log))
{ }

void basic_log::assign_message(message&& msg)
{
    message temp = msg;
    m_logger.output(&temp);
}

} // details
} // log
} // sys