#include "log_mt.h"
#include "threading/threading.h"

namespace sys
{
namespace log
{
namespace details
{

log_mt::log_mt(u64 bufferSize, u64 consumers) :
    log_mt({ }, level::none, bufferSize, consumers)
{ }

log_mt::log_mt(level verbosity, u64 bufferSize, u64 consumers) :
    log_mt({ }, verbosity, bufferSize, consumers)
{ }

log_mt::log_mt(logger&& logger, u64 bufferSize, u64 consumers) :
    log_mt(std::move(logger), level::none, bufferSize, consumers)
{ }

log_mt::log_mt(logger&& logger, level verbosity, u64 bufferSize, u64 consumers) :
    log_manager(verbosity),
    m_logger(std::move(logger)),
    m_queue(bufferSize)
{
    for( u64 consumerIdx = 0; consumerIdx < consumers; consumerIdx++ )
    {
        request_thread(std::format("Log Consumer {}", consumerIdx), [this](){ worker_loop(); }).detach();
    }
}

log_mt::~log_mt()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    flush();
}

void log_mt::assign_message(message&& msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if( !m_queue.push_back(msg) )
    {
        // flush on this thread
        flush();
        m_queue.push_back(msg);
    }
    m_condition.notify_all();
}

void log_mt::worker_loop()
{
    message work;
    while( true )
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [&](){ return !m_queue.empty(); });
            m_queue.pop_front(&work);
        }

        m_logger.output(&work);
    }
}

void log_mt::flush()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    message work;
    while( !m_queue.empty() )
    {
        m_queue.pop_front(&work);
        m_logger.output(&work);
    }
}

} // details
} // log
} // sys