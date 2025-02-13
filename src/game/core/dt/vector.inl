#include "vector.h"

namespace dt
{

template<typename T, typename _allocator>
vector<T, _allocator>::vector() :
    m_data(nullptr),
    m_size(0),
    m_capacity(DT_VECTOR_DEFAULT_CAPACITY)
{
    move_container(m_capacity);
}

template<typename T, typename _allocator>
vector<T, _allocator>::vector(u64 initial_capacity) :
    m_data(nullptr),
    m_size(0),
    m_capacity(initial_capacity)
{
    move_container(m_capacity);
}

template<typename T, typename _allocator>
vector<T, _allocator>::~vector()
{
    for( u64 i = 0; i < m_size; i++ )
    {
        destroy_at(i);
    }
    _allocator::free(m_data);
}

template<typename T, typename _allocator>
T& vector<T, _allocator>::at(u64 index)
{
#if DT_VECTOR_RANGE_CHECK
    DT_ASSERT(index < m_size, "Indexed vector out of bounds. Index {}, Size {}", index, m_size);
#endif

    return m_data[index];
}

template<typename T, typename _allocator>
const T& vector<T, _allocator>::at(u64 index) const
{
#if DT_VECTOR_RANGE_CHECK
    DT_ASSERT(index < m_size, "Indexed vector out of bounds. Index {}, Size {}", index, m_size);
#endif

    return m_data[index];
}

template<typename T, typename _allocator>
T& vector<T, _allocator>::operator[](u64 index)
{
    return at(index);
}

template<typename T, typename _allocator>
const T& vector<T, _allocator>::operator[](u64 index) const
{
    return at(index);
}

template<typename T, typename _allocator>
T& vector<T, _allocator>::front()
{
    return m_data[0];
}

template<typename T, typename _allocator>
const T& vector<T, _allocator>::front() const
{
    return m_data[0];
}

template<typename T, typename _allocator>
T& vector<T, _allocator>::back()
{
    return m_data[m_size - 1];
}

template<typename T, typename _allocator>
const T& vector<T, _allocator>::back() const
{
    return m_data[m_size - 1];
}

template<typename T, typename _allocator>
u64 vector<T, _allocator>::size() const
{
    return m_size;
}

template<typename T, typename _allocator>
u64 vector<T, _allocator>::capacity() const
{
    return m_capacity;
}

template<typename T, typename _allocator>
T* vector<T, _allocator>::data()
{
    return m_data;
}

template<typename T, typename _allocator>
const T* vector<T, _allocator>::data() const
{
    return m_data;
}

template<typename T, typename _allocator>
void vector<T, _allocator>::reserve(u64 new_capacity)
{
    if( new_capacity <= m_capacity )
        return;

    m_capacity = new_capacity;
    move_container(new_capacity);
}

template<typename T, typename _allocator>
void vector<T, _allocator>::resize(u64 new_size)
{
    if( new_size == m_size )
        return;

    if( new_size > m_size )
    {
        // grow container to fit new_size
        u64 growth_amount = new_size - m_size;
        for( u64 i = 0; i < growth_amount - m_size; i++ )
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

template<typename T, typename _allocator>
void vector<T, _allocator>::resize(u64 new_size, const T& value)
{
    if( new_size == m_size )
        return;

    if( new_size > m_size )
    {
        // grow container to fit new_size
        u64 growth_amount = new_size - m_size;
        for( u64 i = 0; i < growth_amount; i++ )
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

template<typename T, typename _allocator>
void vector<T, _allocator>::push_back()
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_DFEAULT_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(m_size++);
}

template<typename T, typename _allocator>
void vector<T, _allocator>::push_back(const T& value)
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(m_size++, value);
}

template<typename T, typename _allocator>
void vector<T, _allocator>::push_back(T&& value)
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(m_size++, std::move(value));
}

template<typename T, typename _allocator>
template<typename... Args>
void vector<T, _allocator>::emplace_back(Args&&... args)
{
    if( m_size == m_capacity )
    {
        m_capacity = DT_VECTOR_GROWTH_EQUATION(m_capacity);
        move_container(m_capacity);
    }

    construct_at(m_size++, std::forward<Args>(args)...);
}

template<typename T, typename _allocator>
void vector<T, _allocator>::erase(u64 index)
{
#if DT_VECTOR_RANGE_CHECK
    DT_ASSERT(index <= m_size, "Erased vector element out of bounds. Index {}, Size {}", index, m_size);
#endif

    destroy_at(index);
    collapse(index, index + 1);
    --m_size;
}

template<typename T, typename _allocator>
void vector<T, _allocator>::shrink_to_size()
{
    if( m_capacity == m_size )
        return;

    m_capacity = m_size;
    move_container(m_capacity);
}

template<typename T, typename _allocator>
void vector<T, _allocator>::move_container(u64 new_size)
{
    T* old_data = m_data;
    m_data = static_cast<T*>(_allocator::allocate(sizeof(T) * new_size, alignof(T)));

    // Move over our old values
    for( u64 i = 0; i < m_size; i++ )
    {
        construct_at(i, std::move(old_data[i]));
    }

    _allocator::free(old_data);
}

template<typename T, typename _allocator>
void vector<T, _allocator>::collapse(u64 left, u64 right)
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

template<typename T, typename _allocator>
void vector<T, _allocator>::construct_at(u64 index, T&& value)
{
    new(&m_data[index]) T(std::move(value));
}

template<typename T, typename _allocator>
void vector<T, _allocator>::construct_at(u64 index, const T& value)
{
    new(&m_data[index]) T(value);
}

template<typename T, typename _allocator>
template<typename... Args>
void vector<T, _allocator>::construct_at(u64 index, Args&&... args)
{
    new(&m_data[index]) T(std::forward<Args>(args)...);
}

template<typename T, typename _allocator>
void vector<T, _allocator>::destroy_at(u64 index)
{
    m_data[index].~T();
}

template<typename T, typename _allocator>
vector<T, _allocator>::iterator vector<T, _allocator>::begin()
{
    return iterator(m_data, 0);
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_iterator vector<T, _allocator>::begin() const
{
    return const_iterator(m_data, 0);
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_iterator vector<T, _allocator>::cbegin() const
{
    return begin();
}

template<typename T, typename _allocator>
vector<T, _allocator>::reverse_iterator vector<T, _allocator>::rbegin()
{
    return reverse_iterator(end());
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_reverse_iterator vector<T, _allocator>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_reverse_iterator vector<T, _allocator>::crbegin() const
{
    return rbegin();
}

template<typename T, typename _allocator>
vector<T, _allocator>::iterator vector<T, _allocator>::end()
{
    return iterator(m_data, m_size);
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_iterator vector<T, _allocator>::end() const
{
    return const_iterator(m_data, m_size);
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_iterator vector<T, _allocator>::cend() const
{
    return end();
}

template<typename T, typename _allocator>
vector<T, _allocator>::reverse_iterator vector<T, _allocator>::rend()
{
    return reverse_iterator(begin());
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_reverse_iterator vector<T, _allocator>::rend() const
{
    return const_reverse_iterator(begin());
}

template<typename T, typename _allocator>
vector<T, _allocator>::const_reverse_iterator vector<T, _allocator>::crend() const
{
    return rend();
}

} // dt