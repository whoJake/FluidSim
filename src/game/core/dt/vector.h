#pragma once
#include "common.h"
#include "array.h"

namespace dt
{

#define DT_VECTOR_DFEAULT_GROWTH_EQUATION(x) (x + (x / 2))
#define DT_VECTOR_DEFAULT_CAPACITY 4

#ifndef DT_VECTOR_GROWTH_EQUATION
    #define DT_VECTOR_GROWTH_EQUATION(x) DT_VECTOR_DFEAULT_GROWTH_EQUATION(x)
#endif

#if DT_VECTOR_DEBUG_LEVEL > 0
    #ifndef DT_VECTOR_RANGE_CHECK
        #define DT_VECTOR_RANGE_CHECK 1
    #endif
#endif

template<typename T, typename _allocator = default_allocator>
class vector
{
public:
    using iterator = array_iterator<T>;
    using const_iterator = const_array_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    vector();
    vector(u64 initial_capacity);

    ~vector();

    // TODO
    // vector(const vector& other);
    // vector(vector&& other);
    // vector<T, _allocator>& operator=(const vector& other);
    // vector<T, _allocator>& operator=(vector&& other);

    T& at(u64 index);
    const T& at(u64 index) const;

    T& operator[](u64 index);
    const T& operator[](u64 index) const;

    T& front();
    const T& front() const;

    T& back();
    const T& back() const;

    u64 size() const;
    u64 capacity() const;

    T* data();
    const T* data() const;

    void reserve(u64 new_capacity);
    void resize(u64 new_size);
    void resize(u64 new_size, const T& value);

    void push_back();
    void push_back(const T& value);
    void push_back(T&& value);

    template<typename... Args>
    void emplace_back(Args&&... args);

    void insert(u64 i, const T& value);
    void insert(u64 i, T&& value);

    void insert(const iterator& it, const T& value);
    void insert(const iterator& it, T&& value);

    void erase(u64 index);
    void shrink_to_size();

    u64 index_of(const const_iterator& it) const;
    u64 index_of(const iterator& it) const;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator crbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_reverse_iterator crend() const;
private:
    void move_container(u64 new_size);

    void collapse(u64 shift_start, u64 new_start);
    void expand(u64 prev_index, u64 new_index);

    void construct_at(u64 index, T&& value);
    void construct_at(u64 index, const T& value);
    
    template<typename... Args>
    void construct_at(u64 index, Args&&... args);

    void destroy_at(u64 index);
private:
    T* m_data;
    u64 m_size;
    u64 m_capacity;
};

} // dt

#ifndef INC_DT_VECTOR_INL

    #define INC_DT_VECTOR_INL
    #include "vector.inl"
#endif