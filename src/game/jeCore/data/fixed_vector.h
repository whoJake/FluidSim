#pragma once

#include "pch/assert.h"

namespace mtl
{

template<typename T>
class fixed_vector
{
public:
    using value_type = T;

// Iterators
public:
    class iterator
    {
    public:
        constexpr iterator() :
            m_pointer()
        { }

        constexpr iterator(T* ptr) :
            m_pointer(ptr)
        { }

        [[nodiscard]]
        constexpr T& operator*() const
        {
            TRAP_EQ(m_pointer, nullptr, "Can't dereference invalid iterator pointer.");

            return *m_pointer;
        }

        [[nodiscard]]
        constexpr T* operator->() const
        {
            return m_pointer;
        }

        constexpr iterator operator++()
        {
            ++m_pointer;
            return *this;
        }

        constexpr iterator& operator++(int)
        {
            iterator temp = *this;
            ++*this;
            return temp;
        }

        constexpr iterator& operator--()
        {
            --m_pointer;
            return *this;
        }

        constexpr iterator operator--(int)
        {
            iterator temp = *this;
            --*this;
            return temp;
        }

        constexpr iterator& operator+=(const std::ptrdiff_t offset)
        {
            m_pointer += offset;
            return *this;
        }

        [[nodiscard]]
        constexpr iterator operator+(const std::ptrdiff_t offset)
        {
            iterator temp = *this;
            temp += offset;
            return temp;
        }

        constexpr iterator& operator-=(const std::ptrdiff_t offset)
        {
            m_pointer -= offset;
            return *this;
        }

        [[nodiscard]]
        constexpr iterator operator-(const std::ptrdiff_t offset)
        {
            iterator temp = *this;
            temp -= offset;
            return temp;
        }

        [[nodiscard]]
        constexpr bool operator==(const iterator& other) const
        {
            return m_pointer == other.m_pointer;
        }

        [[nodiscard]]
        constexpr std::strong_ordering operator<=>(const iterator& other) const
        {
            return m_pointer <=> other.m_pointer;
        }
    private:
        T* m_pointer;
    };

public:
    constexpr fixed_vector(size_t size) :
        fixed_vector(size, T{})
    { }

    constexpr fixed_vector(size_t size, const T& value) :
        m_data(new T[size]),
        m_size(size)
    {
        for( size_t i = 0; i < size; i++ )
        {
            m_data[i] = value;
        }
    }

    constexpr fixed_vector(const std::vector<T>& base) :
        m_data(new T[base.size()]),
        m_size(base.size())
    {
        memcpy(m_data, base.data(), base.size() * sizeof(T));
    }

    constexpr fixed_vector(fixed_vector<T>&& other) :
        m_data(other.m_data),
        m_size(other.m_size)
    {
        other.m_data = nullptr;
    }

    constexpr fixed_vector(const fixed_vector<T>& other) :
        m_data(new T[other.m_size]),
        m_size(other.m_size)
    {
        memcpy(m_data, other.m_data, m_size * sizeof(T));
    }

    constexpr fixed_vector& operator=(fixed_vector<T>&& other)
    {
        TRAP_NEQ(m_size, other.size(), "Cannot move fixed_vector into fixed_vector of different size.");

        m_data = other.m_data;
        other.m_data = nullptr;
        return *this;
    }

    constexpr fixed_vector& operator=(const fixed_vector<T>& other)
    {
        TRAP_NEQ(m_size, other.size(), "Cannot copy fixed_vector into fixed_vector of different size.");

        memcpy(m_data, other.m_data, m_size * sizeof(T));
        return *this;
    }

    constexpr ~fixed_vector()
    {
        if( m_data )
        {
            delete[] m_data;
        }
    }

    constexpr T& operator[](size_t index)
    {
        TRAP_LE(m_size, index, "Index out of bounds.");

        return m_data[index];
    }

    constexpr const T& operator[](size_t index) const
    {
        TRAP_LE(m_size, index, "Index out of bounds.");

        return m_data[index];
    }

    constexpr T& at(size_t index)
    {
        TRAP_LE(m_size, index, "Index out of bounds.");

        return m_data[index];
    }

    constexpr const T& at(size_t index) const
    {
        TRAP_LE(m_size, index, "Index out of bounds.");

        return m_data[index];
    }

    constexpr size_t size() const
    {
        return m_size;
    }

    constexpr void clear(const T& value)
    {
        for( size_t i = 0; i < m_size; i++ )
        {
            m_data[i] = value;
        }
    }

    [[nodiscard]]
    constexpr iterator begin()
    {
        return iterator(m_data);
    }

    [[nodiscard]]
    constexpr iterator end()
    {
        return iterator(m_data + m_size);
    }

    [[nodiscard]]
    constexpr iterator begin() const
    {
        return iterator(m_data);
    }

    [[nodiscard]]
    constexpr iterator end() const
    {
        return iterator(m_data + m_size);
    }

private:
    T* m_data;
    const size_t m_size;
};

} // mtl