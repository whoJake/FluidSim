#pragma once

#include <mutex>

namespace threadsafe
{

template<typename T, size_t capacity>
class Queue
{
public:
    inline bool push_back(const T& item)
    {
        bool retval{ false };

        {
            std::lock_guard<std::mutex> lock(m_lock);
            size_t space = (m_head + 1) % capacity;
            if( space != m_tail )
            {
                m_data[m_head] = item;
                m_head = space;
                retval = true;
            }
        }

        return retval;
    }

    inline bool pop_front(T* item)
    {
        bool retval{ false };

        {
            std::lock_guard<std::mutex> lock(m_lock);
            if( m_tail != m_head )
            {
                *item = m_data[m_tail];
                m_tail = (m_tail + 1) % capacity;
                retval = true;
            }
        }

        return retval;
    }
private:
    T m_data[capacity];
    size_t m_head{ 0 };
    size_t m_tail{ 0 };
    std::mutex m_lock;
};

} // threadsafe