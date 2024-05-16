#include "threading.h"

struct ThreadInfo
{
    uint32_t id;
    std::string name;
};

std::mutex m_mapLock;
std::unordered_map<std::thread::id, ThreadInfo> m_info;
std::mutex m_idLock;
std::vector<uint32_t> m_freeIds;
uint32_t m_activeThreadCount{ 0 };

void destroy_thread(std::thread::id id = std::this_thread::get_id())
{
    std::lock_guard<std::mutex> mapLock(m_mapLock);
    {
        std::lock_guard<std::mutex> idLock(m_idLock);
        m_freeIds.push_back(m_info.at(id).id);
        m_activeThreadCount--;
    }
    m_info.erase(id);
}

std::thread request_thread(std::string name, std::function<void()> function)
{
    uint32_t id{ 0 };
    {
        std::lock_guard<std::mutex> lock(m_idLock);
        if( m_freeIds.empty() )
        {
            id = m_activeThreadCount;
        }
        else
        {
            id = m_freeIds.back();
            m_freeIds.pop_back();
        }
        m_activeThreadCount++;
    }

    std::thread thread([=]{
        function();
        destroy_thread();
    });

    {
        std::lock_guard<std::mutex> lock(m_mapLock);
        m_info.emplace(
            std::piecewise_construct,
            std::tuple(thread.get_id()),
            std::tuple(id, name));
    }

    return thread;
}

uint32_t get_thread_id(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(m_mapLock);
    return m_info.at(id).id;
}

std::string get_thread_name(std::thread::id id)
{
    std::lock_guard<std::mutex> lock(m_mapLock);
    return m_info.at(id).name;
}