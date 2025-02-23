#pragma once
#include "common.h"
#include <type_traits>

namespace dt
{

template<typename T>
class const_array_iterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = u64;
    using pointer = const T*;
    using reference = const T&;

    constexpr const_array_iterator() :
        m_ptr()
    { }

    explicit constexpr const_array_iterator(const T* begin, u64 offset = 0) :
        m_ptr(begin + offset)
    { }

    DEFAULT_MOVE(const_array_iterator);
    DEFAULT_COPY(const_array_iterator);

    constexpr const T* ptr() const
    {
        return m_ptr;
    }

    constexpr const T& operator*() const
    {
        return *m_ptr;
    }

    constexpr const T* operator->() const
    {
        return m_ptr;
    }

    constexpr const_array_iterator& operator++()
    {
        ++m_ptr;
        return *this;
    }

    constexpr const_array_iterator operator++(i32)
    {
        const_array_iterator tmp = *this;
        ++m_ptr;
        return tmp;
    }

    constexpr const_array_iterator& operator--()
    {
        --m_ptr;
        return *this;
    }

    constexpr const_array_iterator operator--(i32)
    {
        const_array_iterator tmp = *this;
        --m_ptr;
        return tmp;
    }

    constexpr const_array_iterator& operator+=(u64 offset)
    {
        m_ptr += offset;
        return *this;
    }

    constexpr const_array_iterator& operator-=(u64 offset)
    {
        m_ptr -= offset;
        return *this;
    }

    constexpr u64 operator-(const const_array_iterator& other) const
    {
        return m_ptr - other.m_ptr;
    }

    constexpr bool operator==(const const_array_iterator& other) const
    {
        return m_ptr == other.m_ptr;
    }

    constexpr std::strong_ordering operator<=>(const const_array_iterator& other) const
    {
        return m_ptr <=> other.m_ptr;
    }

    constexpr const_array_iterator operator+(u64 offset) const
    {
        const_array_iterator tmp = *this;
        tmp += offset;
        return tmp;
    }

    constexpr const_array_iterator operator-(u64 offset) const
    {
        const_array_iterator tmp = *this;
        tmp -= offset;
        return tmp;
    }
private:
    const T* m_ptr;
};

template<typename T>
class array_iterator : public const_array_iterator<T>
{
private:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = u64;
    using pointer = T*;
    using reference = T&;

    using _base = const_array_iterator<T>;
public:
    constexpr array_iterator() :
        _base()
    { }

    explicit constexpr array_iterator(T* begin, u64 offset = 0) :
        _base(begin, offset)
    { }

    DEFAULT_MOVE(array_iterator);
    DEFAULT_COPY(array_iterator);

    constexpr T* ptr() const
    {
        return const_cast<T*>(_base::ptr());
    }

    constexpr T& operator*() const
    {
        return const_cast<T&>(_base::operator*());
    }

    constexpr T* operator->() const
    {
        return const_cast<T*>(_base::operator->());
    }

    constexpr array_iterator& operator++()
    {
        _base::operator++();
        return *this;
    }

    constexpr array_iterator operator++(i32)
    {
        array_iterator tmp = *this;
        _base::operator++();
        return tmp;
    }

    constexpr array_iterator& operator--()
    {
        _base::operator--();
        return *this;
    }

    constexpr array_iterator operator--(i32)
    {
        const_array_iterator tmp = *this;
        _base::operator--();
        return tmp;
    }

    constexpr array_iterator& operator+=(u64 offset)
    {
        _base::operator+=(offset);
        return *this;
    }

    constexpr array_iterator& operator-=(u64 offset)
    {
        _base::operator-=(offset);
        return *this;
    }

    constexpr array_iterator operator+(u64 offset) const
    {
        array_iterator tmp = *this;
        tmp += offset;
        return tmp;
    }

    constexpr array_iterator operator-(u64 offset) const
    {
        array_iterator tmp = *this;
        tmp -= offset;
        return tmp;
    }
};

template<typename T, u32 _capacity>
class inline_array
{
public:
    using const_iterator = const_array_iterator<T>;
    using iterator = array_iterator<T>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;

    static constexpr u32 capacity = _capacity;
    static_assert(capacity != 0, "Inline array cannot be created with zero size.");

    inline_array() = default;
    ~inline_array() = default;

    DEFAULT_MOVE(inline_array);
    DEFAULT_COPY(inline_array);

    constexpr void fill(const T& value)
    {
        for( u32 i = 0; i < _capacity; i++ )
        {
            m_data[i] = value;
        }
    }

    template<typename... Args>
    constexpr void construct_at(u32 index, Args&&... args)
    {
        at(index).~T();
        new(&m_data[index]) T(std::forward<Args>(args)...);
    }

    constexpr T& at(u32 index)
    {
        return m_data[index];
    }

    constexpr const T& at(u32 index) const
    {
        return m_data[index];
    }

    constexpr T& operator[](u32 index)
    {
        return at(index);
    }
    constexpr const T& operator[](u32 index) const
    {
        return at(index);
    }

    constexpr T& front()
    {
        return m_data[0];
    }

    constexpr const T& front() const
    {
        return m_data[0];
    }

    constexpr T& back()
    {
        return m_data[_capacity-1];
    }

    constexpr const T& back() const
    {
        return m_data[_capacity-1];
    }

    constexpr iterator begin()
    {
        return iterator(m_data, 0);
    }

    constexpr reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    constexpr const_iterator begin() const
    {
        return const_iterator(m_data, 0);
    }

    constexpr const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    constexpr const_iterator cbegin() const
    {
        return begin();
    }

    constexpr const_reverse_iterator crbegin() const
    {
        return rbegin();
    }

    constexpr iterator end()
    {
        return iterator(m_data, _capacity);
    }

    constexpr reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }
    
    constexpr const_iterator end() const
    {
        return const_iterator(m_data, _capacity);
    }

    constexpr const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    constexpr const_iterator cend() const
    {
        return end();
    }

    constexpr const_reverse_iterator crend() const
    {
        return rend();
    }

    constexpr u32 size() const
    {
        return _capacity;
    }

    constexpr T* data()
    {
        return m_data;
    }

    constexpr const T* data() const
    {
        return m_data;
    }
private:
    T m_data[_capacity];
};

template<typename T, typename _allocator = default_allocator>
class array
{
public:
    using iterator = array_iterator<T>;
    using const_iterator = const_array_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    array() :
        m_data(nullptr),
        m_capacity(0)
    { }

    array(u64 capacity) :
        array()
    {
        initialise(capacity);
    }

    ~array()
    {
        kill();
    }

    array(array&& other) :
        m_data(other.m_data),
        m_capacity(other.m_capacity)
    {
        other.m_data = nullptr;
        other.m_capacity = 0;
    }

    array& operator=(array&& other)
    {
        m_data = other.m_data;
        m_capacity = other.m_capacity;
        other.m_data = nullptr;
        other.m_capacity = 0;

        return *this;
    }

    array(const array& other)
    {
        initialise(other.size());
        for( u64 i = 0; i < m_capacity; i++ )
        {
            m_data[i] = other.m_data[i];
        }
    }

    array& operator=(const array& other)
    {
        if( m_capacity == other.m_capacity )
        {
            // We don't have to re-allocate our own
            // array so do a small optimization here.
            kill(false);
        }
        else
        {
            kill();
            initialise(other.m_capacity, false);
        }

        for( u64 i = 0; i < m_capacity; i++ )
        {
            m_data[i] = other.m_data[i];
        }

        return *this;
    }

    inline void initialise(u64 capacity, bool construct = true)
    {
        DT_ASSERT(!m_data, "Array is already initialised.");

        m_capacity = capacity;
        if( capacity != 0 )
            m_data = static_cast<T*>(_allocator::allocate(sizeof(T) * capacity, alignof(T)));

        if( !construct )
            return;

        for( u64 i = 0; i < capacity; i++ )
        {
            new(&m_data[i]) T();
        }
    }

    inline T& at(u64 index)
    {
        return m_data[index];
    }

    inline const T& at(u64 index) const
    {
        return m_data[index];
    }

    inline T& operator[](u64 index)
    {
        return at(index);
    }

    inline const T& operator[](u64 index) const
    {
        return at(index);
    }

    inline T& front()
    {
        return at(0);
    }

    inline const T& front() const
    {
        return at(0);
    }

    inline T& back()
    {
        return at(m_capacity - 1);
    }
    inline const T& back() const
    {
        return at(m_capacity - 1);
    }

    inline u64 size() const
    {
        return m_capacity;
    }

    inline T* data()
    {
        return m_data;
    }

    inline const T* data() const
    {
        return m_data;
    }

    inline u64 index_of(const const_iterator& it) const
    {
        return it.ptr() - m_data;
    }

    inline u64 index_of(const iterator& it) const
    {
        return it.ptr() - m_data;
    }

    inline iterator begin()
    {
        return iterator(m_data, 0);
    }

    inline reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    inline const_iterator begin() const
    {
        return const_iterator(m_data, 0);
    }

    inline const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    inline const_iterator cbegin() const
    {
        return begin();
    }

    inline const_reverse_iterator crbegin() const
    {
        return rbegin();
    }

    inline iterator end()
    {
        return iterator(m_data, m_capacity);
    }

    inline reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    inline const_iterator end() const
    {
        return const_iterator(m_data, m_capacity);
    }

    inline const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    inline const_iterator cend() const
    {
        return end();
    }

    inline const_reverse_iterator crend() const
    {
        return rend();
    }
private:
    inline void kill(bool deallocate = true)
    {
        for( u64 i = 0; i < m_capacity; i++ )
        {
            m_data[i].~T();
        }

        if( deallocate )
            _allocator::free(m_data);
    }
private:
    T* m_data;
    u64 m_capacity;
};

} // dt