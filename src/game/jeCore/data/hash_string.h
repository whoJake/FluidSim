#pragma once
#include <string>

#include <ostream>
#include <format>

namespace mtl
{

class hash_string
{
public:
    static constexpr size_t hash_function(std::string_view string)
    {
        // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes ?
        size_t seed = 0x517cc1b727220a95;
        for( char c : string )
        {
            std::hash<char> hasher;
            size_t hash = hasher(c);

            hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash;
        }
        return seed;
    }

    static constexpr size_t hash_function(std::wstring_view wstring)
    {
        // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes ?
        size_t seed = 0x517cc1b727220a95;
        for( wchar_t c : wstring )
        {
            std::hash<wchar_t> hasher;
            size_t hash = hasher(c);

            hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash;
        }
        return seed;
    }
public:
    constexpr explicit hash_string(size_t precalculated_hash) :
        m_hash(precalculated_hash)
    #ifdef CFG_DEBUG
        , m_debugCopy()
    #endif
    { }

    constexpr explicit hash_string(const char* string) :
        m_hash(hash_function(string))
    #ifdef CFG_DEBUG
        , m_debugCopy(string)
    #endif
    { }

    constexpr explicit hash_string(const wchar_t* string) :
        m_hash(hash_function(string))
    #ifdef CFG_DEBUG
        , m_debugCopy()
    #endif
    { }

    constexpr explicit hash_string(const std::string& string) :
        m_hash(hash_function(string))
    #ifdef CFG_DEBUG
        , m_debugCopy(string)
    #endif
    { }

    constexpr explicit hash_string(const std::wstring& string) :
        m_hash(hash_function(string))
    #ifdef CFG_DEBUG
        , m_debugCopy()
    #endif
    { }

    constexpr explicit hash_string(std::string_view string) :
        m_hash(hash_function(string))
    #ifdef CFG_DEBUG
        , m_debugCopy(string)
    #endif
    { }

    constexpr explicit hash_string(std::wstring_view string) :
        m_hash(hash_function(string))
    #ifdef CFG_DEBUG
        , m_debugCopy()
    #endif
    { }

    constexpr ~hash_string() = default;

    inline constexpr std::strong_ordering operator<=>(const hash_string& other) const = default;

    friend std::ostream& operator<<(std::ostream& stream, const hash_string& output)
    {
        stream << std::format("{:x}", output.get_hash());
    #ifdef CFG_DEBUG
        stream << std::format(" ({})", output.m_debugCopy);
    #endif
    }

    inline constexpr size_t get_hash() const
    {
        return m_hash;
    }
private:
    size_t m_hash;
#ifdef CFG_DEBUG
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