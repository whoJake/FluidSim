#pragma once

#include <functional>
#include <atomic>
#include "Queue.h"
#include "threading.h"

struct DispatchState
{
    uint32_t groupIndex;
    uint32_t jobIndex;
    uint32_t jobGroupIndex;
};

enum WorkerState
{
    UNINITIALIZED = 0,
    IDLE,
    WORKING
};

struct WorkerInfo
{
    std::thread::id id{ };
    WorkerState state{ UNINITIALIZED };
};

class JobDispatch
{
public:
    static void initialize();

    static size_t get_worker_count();

    [[nodiscard]] 
    static std::atomic<uint32_t>* execute(const std::function<void()>& job);
    static void execute_and_wait(const std::function<void()>& job);

    [[nodiscard]] 
    static std::atomic<uint32_t>* dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(DispatchState)>& job);
    static void dispatch_and_wait(uint32_t jobCount, uint32_t groupSize, const std::function<void(DispatchState)>& job);

    static void reset_counters();

    static void poll();
private:
    static std::atomic<uint32_t>* request_atomic_counter(uint32_t initialValue);
private:
    static JobDispatch& instance();
    static JobDispatch* m_instance;
private:
    threadsafe::Queue<std::function<void()>, 256> m_jobPool{ };
    std::vector<WorkerInfo> m_workers{ };

    std::mutex m_wakeMutex;
    std::condition_variable m_wakeCondition;

    std::unordered_set<std::atomic<uint32_t>*> m_counters{ };
};