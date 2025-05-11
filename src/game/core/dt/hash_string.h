#pragma once
#include "system/assert.h"
#include "shared.h"
#include "vector.h"

namespace dt
{

SYSDECLARE_CHANNEL(hash_string);
#define HASHSTR_ASSERT(cond, fmt, ...) SYSASSERT(cond, SYSMSG_CHANNEL_FATAL(hash_string, fmt, __VA_ARGS__))

enum hash_string_type
{
    HASH_STR_TYPE_DEFAULT,
    HASH_STR_TYPE_COUNT,
};

template<typename _underlying>
class hash_string_table
{
public:
    hash_string_table() = default;
    ~hash_string_table() = default;

    void insert(_underlying hash, std::string&& str);
    void remove(_underlying hash);

    bool has(_underlying hash) const;
    std::string_view get_str(_underlying hash) const;
private:
    u64 find(_underlying hash) const;
private:
    dt::vector<_underlying> m_hashes;
    dt::vector<std::string> m_strings;
};

template<typename _underlying, hash_string_type _type>
class basic_hash_string
{
public:
    basic_hash_string(std::string_view str, _underlying hash);
    basic_hash_string(std::string_view str);
    basic_hash_string(_underlying hash = 0);

    ~basic_hash_string();

    DEFAULT_MOVE(basic_hash_string);
    DEFAULT_COPY(basic_hash_string);

    _underlying get_hash() const;
    std::string_view try_get_str() const;

    inline std::strong_ordering operator<=>(const basic_hash_string& rhs) const
    {
        return get_hash() <=> rhs.get_hash();
    }

    inline bool operator!=(const basic_hash_string& rhs)
    {
        return get_hash() != rhs.get_hash();
    }
private:
    static _underlying calculate(std::string_view str);
    static hash_string_table<_underlying>& get_table();
private:
    _underlying m_hash;
};

using hash_string32 = basic_hash_string<u32, HASH_STR_TYPE_DEFAULT>;
using hash_string64 = basic_hash_string<u64, HASH_STR_TYPE_DEFAULT>;

} // dt

#ifndef INC_DT_HASH_STRING_INL
    #define INC_DT_HASH_STRING_INL
    #include "hash_string.inl"
#endif