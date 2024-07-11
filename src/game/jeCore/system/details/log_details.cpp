#include "log_details.h"

namespace sys
{
namespace log
{
namespace details
{

logger::logger() :
    m_targets()
{ }

logger::logger(logger&& other) noexcept :
    m_targets(std::move(other.m_targets))
{
    other.m_targets = { };
}

logger& logger::operator=(logger&& other) noexcept
{
    m_targets = std::move(other.m_targets);
    other.m_targets = { };
    return *this;
}

logger::~logger()
{
    for( target* ptarget : m_targets )
    {
        delete ptarget;
    }

    m_targets.clear();
}

void logger::output(message* msg)
{
    for( target* ptarget : m_targets )
    {
        ptarget->output(msg);
    }
}

void logger::register_target(target* ptarget)
{
    m_targets.push_back(ptarget);
}

log_manager::log_manager(level verbosity) :
    m_verbosity(verbosity)
{ }

void log_manager::add_message(message&& msg)
{
    if( u8_cast(msg.lvl) >= u8_cast(m_verbosity) )
    {
        assign_message(std::move(msg));
    }
}

} // details
} // log
} // sys