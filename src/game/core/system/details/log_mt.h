#pragma once

#include "log_details.h"
#include "data/queue.h"
#include <mutex>
#include <condition_variable>

namespace sys
{
namespace log
{
namespace details
{

class log_mt : public log_manager
{
public:
    log_mt(u64 bufferSize = 1024, u64 consumers = 1);
    log_mt(level verbosity = level::none, u64 bufferSize = 1024, u64 consumers = 1);
    log_mt(logger&& logger, u64 bufferSize = 1024, u64 consumers = 1);
    log_mt(logger&& logger, level verbosity = level::none, u64 bufferSize = 1024, u64 consumers = 1);
    ~log_mt();

    void assign_message(message&& msg) override;

    void flush() override;
private:
    void worker_loop();
private:
    logger m_logger;
    mtl::queue_v<message> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
};

} // details
} // log
} // sys