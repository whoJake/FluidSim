#include "JobDispatcher.h"

#include <thread>

JobDispatch* JobDispatch::m_instance = nullptr;


#define DEFAULT_WORKER_THREADS 4
PARAM(worker_threads);
PARAM(detect_worker_thread_count);

void JobDispatch::initialize()
{
    if( m_instance )
    {
        QUITFMT("JobDispatch has already been initialized.");
    }
    m_instance = new JobDispatch();

    uint32_t workers{ 0 };
    if( Param_detect_worker_thread_count.get() )
    {
        uint32_t hardwareThreadsAvailable = std::thread::hardware_concurrency();
        workers = std::max(1u, hardwareThreadsAvailable - 1);
    }
    else
    {
        workers = DEFAULT_WORKER_THREADS;
        Param_worker_threads.get_int((int*) &workers);
        workers = std::max(1u, workers);
    }

    // thread data
    threadsafe::Queue<std::function<void()>, 256>& pool = instance().m_jobPool;
    std::mutex& wakeMutex = instance().m_wakeMutex;
    std::condition_variable& wakeCondition = instance().m_wakeCondition;

    instance().m_workers.resize(workers);
    for( uint32_t i = 0; i < workers; i++ )
    {
        // data
        WorkerInfo& info = instance().m_workers.at(i);
        std::string workerName(std::format("WORKER_{}", i));

        std::thread worker = request_thread(workerName, [&]{
                std::function<void()> activeJob;
                info.state = IDLE;

                while( true )
                {
                    if( pool.pop_front(&activeJob) )
                    {
                        info.state = WORKING;
                        activeJob();
                    }
                    else
                    {
                        info.state = IDLE;
                        std::unique_lock<std::mutex> lock(wakeMutex);
                        wakeCondition.wait(lock);
                    }
                }
        });

        info.id = worker.get_id();
        auto pair = instance().m_threadLogs.emplace(
            std::piecewise_construct,
            std::tuple(info.id),
            std::tuple());

        std::string logFilename = std::format("logs/{}.txt", workerName.c_str());
        std::remove(logFilename.c_str());
        pair.first->second.register_target(new jclog::FileTarget(logFilename.c_str()));

        worker.detach();
    }
}

size_t JobDispatch::get_worker_count()
{
    return instance().m_workers.size();
}

void JobDispatch::poll()
{
    instance().m_wakeCondition.notify_one();
    std::this_thread::yield();
}

std::atomic<uint32_t>* JobDispatch::request_atomic_counter(uint32_t initialValue)
{
    auto result = instance().m_counters.emplace(new std::atomic<uint32_t>(initialValue));
    return *(result.first);
}

void JobDispatch::reset_counters()
{
    for( std::atomic<uint32_t>* counter : instance().m_counters )
    {
        delete counter;
    }

    instance().m_counters.clear();
}

jclog::Log& JobDispatch::get_thread_log(std::thread::id tid)
{
    return instance().m_threadLogs.at(tid);
}

JobDispatch& JobDispatch::instance()
{
    return *m_instance;
}

std::atomic<uint32_t>* JobDispatch::execute(const std::function<void()>& job)
{
    std::atomic<uint32_t>* retval = request_atomic_counter(1u);

    std::function<void()> trackedJob = [retval, &job]{
        job();
        (*retval)--;
    };

    while( !instance().m_jobPool.push_back(trackedJob) )
    {
        poll();
    }
    instance().m_wakeCondition.notify_one();

    return retval;
}

void JobDispatch::execute_and_wait(const std::function<void()>& job)
{
    std::atomic<uint32_t>* counter = execute(job);
    while( counter->load() != 0u )
    {
        poll();
    }
    instance().m_wakeCondition.notify_one();
    return;
}

std::atomic<uint32_t>* JobDispatch::dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(DispatchState)>& job)
{
    std::atomic<uint32_t>* retval = request_atomic_counter(jobCount);

    if( jobCount == 0 || groupSize == 0 )
    {
        retval->store(0u);
        return retval;
    }

    uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

    for( uint32_t i = 0; i < groupCount; i++ )
    {
        std::function<void()> groupJob = [=, &job](){
            DispatchState state{ };
            state.groupIndex = i;

            uint32_t groupStartIndex = groupSize * i;
            uint32_t groupEndIndex = std::min(groupStartIndex + groupSize, jobCount);

            for( uint32_t jobGroupIndex = 0; jobGroupIndex < groupEndIndex - groupStartIndex; jobGroupIndex++ )
            {
                state.jobGroupIndex = jobGroupIndex;
                state.jobIndex = groupStartIndex + jobGroupIndex;

                job(state);
                (*retval)--;
            }

        };

        while( !instance().m_jobPool.push_back(groupJob) )
        {
            poll();
        }

        instance().m_wakeCondition.notify_one();
    }

    return retval;
}

void JobDispatch::dispatch_and_wait(uint32_t jobCount, uint32_t groupSize, const std::function<void(DispatchState)>& job)
{
    std::atomic<uint32_t>* counter = dispatch(jobCount, groupSize, job);
    while( counter->load() != 0u )
    {
        poll();
    }
    instance().m_wakeCondition.notify_one();
    return;
}