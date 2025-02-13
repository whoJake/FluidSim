#include "unique_ptr.h"

namespace dt
{

template<typename T, typename _allocator>
unique_ptr<T, _allocator>::unique_ptr() :
    m_ptr(nullptr)
{ }

template<typename T, typename _allocator>
unique_ptr<T, _allocator>::unique_ptr(T* ptr) :
    m_ptr(ptr)
{ }

template<typename T, typename _allocator>
unique_ptr<T, _allocator>::unique_ptr(unique_ptr&& other) :
    m_ptr(nullptr)
{
    reset(other.release());
}

template<typename T, typename _allocator>
unique_ptr<T, _allocator>& unique_ptr<T, _allocator>::operator=(unique_ptr&& other)
{
    reset(other.release());
    return *this;
}

template<typename T, typename _allocator>
T* unique_ptr<T, _allocator>::release()
{
    T* tmp = m_ptr;
    m_ptr = nullptr;
    return tmp;
}

template<typename T, typename _allocator>
T* unique_ptr<T, _allocator>::get() const
{
    return m_ptr;
}

template<typename T, typename _allocator>
template<typename... Args>
void unique_ptr<T, _allocator>::make_new(Args&&... args)
{
    T* new_ptr = static_cast<T*>(_allocator::allocate(sizeof(T), alignof(T)));
    new(new_ptr) T(std::forward<Args>(args)...);
    reset(new_ptr);
}

template<typename T, typename _allocator>
void unique_ptr<T, _allocator>::reset(T* replacement)
{
    if( m_ptr )
    {
        m_ptr->~T();
        _allocator::free(m_ptr);
    }

    m_ptr = replacement;
}

template<typename T, typename _allocator>
void unique_ptr<T, _allocator>::swap(unique_ptr& other)
{
    T* tmp = m_ptr;
    m_ptr = other.release();
    other.reset(tmp);
}

template<typename T, typename _allocator>
T& unique_ptr<T, _allocator>::operator*() const
{
    return *m_ptr;
}

template<typename T, typename _allocator>
T* unique_ptr<T, _allocator>::operator->() const
{
    return m_ptr;
}

} // dt