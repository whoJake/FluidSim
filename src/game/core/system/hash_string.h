#pragma once
#include "hash.h"

namespace sys
{

enum class hash_string_pools
{
    default_pool = 0,
    pool_count,
};

// #define SYS_HASH_STRING_32_BIT

#ifdef SYS_HASH_STRING_32_BIT
using hash_string_value = u32;
#define CALCULATE_HASH(stringview) ::sys::hash32(stringview.data(), stringview.size())
#else
using hash_string_value = u64;
#define CALCULATE_HASH(stringview) ::sys::hash64(stringview.data(), stringview.size())
#endif

class hash_string_table
{
public:
    template<bool, hash_string_pools, bool>
    friend class basic_hash_string;
private:
    static void insert(hash_string_pools pool, hash_string_value hash, std::string_view string);
    static std::string_view lookup(hash_string_pools pool, hash_string_value hash);
};

template<bool case_insensitive, hash_string_pools pool, bool use_table>
class basic_hash_string
{
public:
    explicit basic_hash_string(hash_string_value precalculated_hash) :
        m_hash(precalculated_hash)
    { }

    basic_hash_string(std::string_view string) :
        m_hash(calculate(string))
    { }

    DEFAULT_COPY(basic_hash_string);
    DEFAULT_MOVE(basic_hash_string);

    hash_string_value get_hash() const
    {
        return m_hash;
    }

    std::string_view try_get_str() const
    {
        if constexpr( use_table )
        {
            return hash_string_table::lookup(pool, m_hash);
        }
        else
        {
            return std::string_view("");
        }
    }
private:
    static hash_string_value calculate(std::string_view string)
    {
        // TODO case insensitive
        hash_string_value val = CALCULATE_HASH(string);
        if constexpr( use_table )
        {
            hash_string_table::insert(pool, val, string);
        }
        return val;
    }
private:
    hash_string_value m_hash;
};

template<hash_string_pools pool>
#ifdef SYS_HASH_STRING_CASE_SENSITIVE
using pooled_hash_string = basic_hash_string<false, pool, true>;
#else
using pooled_hash_string = basic_hash_string<true, pool, true>;
#endif

using hash_string = pooled_hash_string<hash_string_pools::default_pool>;

} // sys