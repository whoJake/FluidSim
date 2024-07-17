#pragma once
#include <string>

#include <ostream>
#include <format>

namespace mtl
{

class hash_string
{
public:
    static size_t hash_function(std::string_view string)
    {
        std::hash<std::string_view> hasher;
        return hasher(string);
    }

    static size_t hash_function(std::wstring_view wstring)
    {
        std::hash<std::wstring_view> hasher;
        return hasher(wstring);
    }
public:
    explicit hash_string(size_t precalculated_hash) :
        m_hash(precalculated_hash)
    #ifndef CFG_FINAL
        , m_debugCopy()
    #endif
    { }

    explicit hash_string(const char* string) :
        m_hash(hash_function(string))
    #ifndef CFG_FINAL
        , m_debugCopy(string)
    #endif
    { }

    explicit hash_string(const wchar_t* string) :
        m_hash(hash_function(string))
    #ifndef CFG_FINAL
        , m_debugCopy()
    #endif
    { }

    explicit hash_string(const std::string& string) :
        m_hash(hash_function(string))
    #ifndef CFG_FINAL
        , m_debugCopy(string)
    #endif
    { }

    explicit hash_string(const std::wstring& string) :
        m_hash(hash_function(string))
    #ifndef CFG_FINAL
        , m_debugCopy()
    #endif
    { }

    explicit hash_string(std::string_view string) :
        m_hash(hash_function(string))
    #ifndef CFG_FINAL
        , m_debugCopy(string)
    #endif
    { }

    explicit hash_string(std::wstring_view string) :
        m_hash(hash_function(string))
    #ifndef CFG_FINAL
        , m_debugCopy()
    #endif
    { }

    explicit hash_string(hash_string&& other) noexcept :
        m_hash(other.m_hash)
    #ifndef CFG_FINAL
        , m_debugCopy(std::move(other.m_debugCopy))
    #endif
    {
        other.m_hash = 0;
    #ifndef CFG_FINAL
        m_debugCopy = { };
    #endif
    }

    explicit hash_string(const hash_string& other) :
        m_hash(other.m_hash)
    #ifndef CFG_FINAL
        , m_debugCopy(other.m_debugCopy)
    #endif
    { }

    hash_string& operator=(hash_string&& other) noexcept
    {
        m_hash = other.m_hash;
        other.m_hash = 0;
    #ifndef CFG_FINAL
        m_debugCopy = std::move(other.m_debugCopy);
        other.m_debugCopy = { };
    #endif

        return *this;
    }

    hash_string& operator=(const hash_string& other)
    {
        m_hash = other.m_hash;
    #ifndef CFG_FINAL
        m_debugCopy = other.m_debugCopy;
    #endif
        return *this;
    }

    ~hash_string() = default;

    inline std::strong_ordering operator<=>(const hash_string& other) const = default;

    friend std::ostream& operator<<(std::ostream& stream, const hash_string& output)
    {
        stream << std::format("{:x}", output.get_hash());
    #ifndef CFG_FINAL
        stream << std::format(" ({})", output.m_debugCopy);
    #endif
    }

    inline size_t get_hash() const
    {
        return m_hash;
    }
private:
    size_t m_hash;
#ifndef CFG_FINAL
    std::string m_debugCopy;
#endif
};

} // mtl

namespace std
{

template<>
struct hash<mtl::hash_string>
{
    size_t operator()(const mtl::hash_string& obj) const
    {
        return obj.get_hash();
    }
};

} // std