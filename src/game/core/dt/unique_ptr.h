#pragma once
#include "shared.h"

namespace dt
{

template<typename T, typename _allocator = default_allocator>
class unique_ptr
{
public:
    unique_ptr();
    unique_ptr(T* ptr);

    unique_ptr(unique_ptr&& other);
    unique_ptr& operator=(unique_ptr&& other);
    
    DELETE_COPY(unique_ptr);

    T* release();
    T* get() const;

    template<typename... Args>
    void make_new(Args&&... args);

    void reset(T* replacement = nullptr);
    void swap(unique_ptr& other);

    T& operator*() const;
    T* operator->() const;
private:
    T* m_ptr;
};

template<typename T, typename _allocator = default_allocator>
inline unique_ptr<T, _allocator> make_unique(T* ptr)
{
    return unique_ptr<T, _allocator>(ptr);
}

template<typename T, typename _allocator = default_allocator, typename... Args>
inline unique_ptr<T, _allocator> make_unique(Args&&... args)
{
    T* new_ptr = static_cast<T*>(_allocator::allocate(sizeof(T), alignof(T)));
    new(new_ptr) T(std::forward<Args>(args)...);
    return unique_ptr<T, _allocator>(new_ptr);
}

} // dt

#ifndef INC_DT_UNIQUE_PTR_INL
    #define INC_DT_UNIQUE_PTR_INL
    #include "unique_ptr.inl"
#endif