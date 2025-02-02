#include "vector.h"

namespace dt
{

template<typename T>
vector<T>::vector(allocator* alloc_method) :
    m_data(nullptr),
    m_allocator(alloc_method),
    m_size(0),
    m_capacity(DT_VECTOR_DEFAULT_CAPACITY)
{
    move_container(m_capacity);
}

template<typename T>
vector<T>::vector(u64 initial_capacity, allocator* alloc_method) :
    m_data(nullptr),
    m_allocator(alloc_method),
    m_size(0),
    m_capacity(initial_capacity)
{
    move_container(m_capacity);
}

template<typename T>
vector<T>::~vector()
{
    for( u64 i = 0; i < m_size; i++ )
    {
        destroy_at(i);
    }
    m_allocator->free(m_data);
}

template<typename T>
T& vector<T>::at(u64 index)
{
#if DT_VECTOR_RANGE_CHECK
    DT_ASSERT(index < m_size, "Indexed vector out of bounds. Index {}, Size {}", index, m_size);
#endif

    return m_data[index];
}

template<typename T>
const T& vector<T>::at(u64 index) const
{
#if DT_VECTOR_RANGE_CHECK
    DT_ASSERT(index < m_size, "Indexed vector out of bounds. Index {}, Size {}", index, m_size);
#endif

    return m_data[index];
}

template<typename T>
T& vector<T>::operator[](u64 index)
{
    return at(index);
}

template<typename T>
const T& vector<T>::operator[](u64 index) const
{
    return at(index);
}

template<typename T>
T& vector<T>::front()
{
    return m_data[0];
}

template<typename T>
const T& vector<T>::front() const
{
    return m_data[0];
}

template<typename T>
T& vector<T>::back()
{
    return m_data[m_size - 1];
}

template<typename T>
const T& vector<T>::back() const
{
    return m_data[m_size - 1];
}

template<typename T>
u64 vector<T>::size() const
{
    return m_size;
}

template<typename T>
u64 vector<T>::capacity() const
{
    return m_capacity;
}

template<typename T>
T* vector<T>::data()
{
    return m_data;
}

template<typename T>
const T* vector<T>::data() const
{
    return m_data;
}

template<typename T>
void vector<T>::reserve(u64 new_capacity)
{
    if( new_capacity <= m_capacity )
        return;

    m_capacity = new_capacity;
    move_container(new_capacity);
}

template<typename T>
void vector<T>::resize(u64 new_size)
{
    if( new_size == m_size )
        return;

    if( new_size > m_size )
    {
        // grow container to fit new_size
        for( u64 i = 0; i < new_size - m_size; i++ )
        {
            // Using emplace_back to default construct +
            // adher to normal growth rules.
            emplace_back();
        }
    }
    else
    {
        // new_size < m_size
        // shrink container to new_size, destructing everything
        for( u64 i = m_size; i > new_size; i-- )
        {
            erase(i - 1);
        }
    }
}

template<typename T>
void vector<T>::resize(u64 new_size, const T& value)
{
    if( new_size == m_size )
        return;

    if( new_size > m_size )
    {
        // grow container to fit new_size
        for( u64 i = 0; i < new_size - m_size; i++ )
        {
            // Using emplace_back to default construct +
            // adher to normal growth rules.
            push_back(value);
        }
    }
    else
    {
        // new_size < m_size
        // shrink container to new_size
        for( u64 i = m_size; i > new_size; i-- )
        {
            erase(i - 1);
        }
    }
}

template<typename T>
void vector<T>::push_back()
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_DFEAULT_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(m_size++);
}

template<typename T>
void vector<T>::push_back(const T& value)
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(m_size++, value);
}

template<typename T>
void vector<T>::push_back(T&& value)
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(std::move(value));
}

template<typename T>
template<typename... Args>
void vector<T>::emplace_back(Args&&... args)
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(std::forward<Args>(args)...);
}

template<typename T>
void vector<T>::erase(u64 index)
{
#if DT_VECTOR_RANGE_CHECK
    DT_ASSERT(index <= m_size, "Erased vector element out of bounds. Index {}, Size {}", index, m_size);
#endif

    destroy_at(index);
    collapse(index, index + 1);
    --m_size;
}

template<typename T>
void vector<T>::shrink_to_size()
{
    if( m_capacity == m_size )
        return;

    m_capacity = m_size;
    move_container(m_capacity);
}

template<typename T>
void vector<T>::move_container(u64 new_size)
{
    T* old_data = m_data;
    m_data = static_cast<T*>(m_allocator->allocate(sizeof(T) * new_size, alignof(T)));

    // Move over our old values
    for( u64 i = 0; i < m_size; i++ )
    {
        construct_at(i, std::move(old_data[i]));
    }

    m_allocator->free(old_data);
}

template<typename T>
void vector<T>::collapse(u64 left, u64 right)
{
    u64 diff = right - left;
    for( u64 i = right; i < m_size; i++ )
    {
        // Move pattern like so
        // 1: x o o x x x c
        // 2: x x o o x x c
        // 3: x x x o o x c
        // 4: x x x x o o c
        construct_at(i - diff, std::move(m_data[i]));
    }
}

template<typename T>
void vector<T>::construct_at(u64 index, T&& value)
{
    new(&m_data[index]) T(std::move(value));
}

template<typename T>
void vector<T>::construct_at(u64 index, const T& value)
{
    new(&m_data[index]) T(value);
}

template<typename T>
template<typename... Args>
void vector<T>::construct_at(u64 index, Args&&... args)
{
    new(&m_data[index]) T(std::forward<Args>(args)...);
}

template<typename T>
void vector<T>::destroy_at(u64 index)
{
    m_data[index].~T();
}

template<typename T>
vector<T>::iterator vector<T>::begin()
{
    return iterator(m_data, 0);
}

template<typename T>
vector<T>::const_iterator vector<T>::begin() const
{
    return const_iterator(m_data, 0);
}

template<typename T>
vector<T>::const_iterator vector<T>::cbegin() const
{
    return begin();
}

template<typename T>
vector<T>::reverse_iterator vector<T>::rbegin()
{
    return reverse_iterator(end());
}

template<typename T>
vector<T>::const_reverse_iterator vector<T>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<typename T>
vector<T>::const_reverse_iterator vector<T>::crbegin() const
{
    return rbegin();
}

template<typename T>
vector<T>::iterator vector<T>::end()
{
    return iterator(m_data, m_size);
}

template<typename T>
vector<T>::const_iterator vector<T>::end() const
{
    return const_iterator(m_data, m_size);
}

template<typename T>
vector<T>::const_iterator vector<T>::cend() const
{
    return end();
}

template<typename T>
vector<T>::reverse_iterator vector<T>::rend()
{
    return reverse_iterator(begin());
}

template<typename T>
vector<T>::const_reverse_iterator vector<T>::rend() const
{
    return const_reverse_iterator(begin());
}

template<typename T>
vector<T>::const_reverse_iterator vector<T>::crend() const
{
    return rend();
}

} // dt