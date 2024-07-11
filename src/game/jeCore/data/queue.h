#pragma once

#include "fixed_vector.h"

namespace mtl
{

template<typename T>
class queue_v
{
public:
    queue_v(u64 capacity) :
        m_data(capacity)
    { }

    queue_v() :
        queue_v(0)
    { }

    queue_v(queue_v&& other) :
        m_data(std::move(other.m_data)),
        m_head(other.m_head),
        m_tail(other.m_tail)
    { }

    queue_v& operator=(queue_v&& other)
    {
        m_data = std::move(other.m_data);
        m_head = other.m_head;
        m_tail = other.m_tail;
        return *this;
    }

    inline bool empty() const
    {
        return m_head == m_tail;
    }

    inline bool push_back(const T& item)
    {
        size_t space = (m_head + 1) % m_data.size();
        if( space != m_tail )
        {
            m_data[m_head] = item;
            m_head = space;
            return true;
        }
        else
        {
            // no room in queue
            return false;
        }
    }

    inline bool push_back(T&& item)
    {
        size_t space = (m_head + 1) % m_data.size();
        if( space != m_tail )
        {
            m_data[m_head] = std::move(item);
            m_head = space;
            return true;
        }
        else
        {
            // no room in queue
            return false;
        }
    }

    inline bool pop_front(T* item)
    {
        if( m_tail != m_head )
        {
            *item = m_data[m_tail];
            m_tail = (m_tail + 1) % m_data.size();
            return true;
        }
        else
        {
            // nothing to pop
            return false;
        }
    }

    inline bool pop_front(T** item)
    {
        if( m_tail != m_head )
        {
            *item = &m_data[m_tail];
            m_tail = (m_tail + 1) % m_data.size();
            return true;
        }
        else
        {
            // nothing to pop
            return false;
        }
    }
private:
    mtl::fixed_vector<T> m_data;
    u64 m_head{ 0 };
    u64 m_tail{ 0 };
};

} // mtl