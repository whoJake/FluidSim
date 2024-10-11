#pragma once

#include "data/fixed_vector.h"
#include <mutex>

namespace mtl
{
namespace ts
{

template<typename T>
class queue_v
{
public:
    queue_v() :
        m_data()
    { }

    queue_v(u64 capacity) :
        m_data(capacity)
    { }

    inline bool push_back(const T& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        u64 space = (m_head + 1) % m_data.size();
        if( space != m_tail )
        {
            m_data[m_head] = item;
            m_head = space;
            return true;
        }
        return false;
    }

    inline bool pop_front(T* item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if( m_tail != m_head )
        {
            u64 n = (m_tail + 1) % m_data.size();
            *item = m_data[n];
            m_tail = n;
            return true;
        }
        return false;
    }

private:
    mtl::fixed_vector<T> m_data;
    u64 m_head{ 0 };
    u64 m_tail{ 0 };
    std::mutex m_mutex;
};

} // ts
} // mtl