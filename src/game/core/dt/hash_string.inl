#include "hash_string.h"
#include "system/hashing/spookyhash.h"

namespace dt
{

template<typename _underlying, hash_string_type _type>
basic_hash_string<_underlying, _type>::basic_hash_string(std::string_view str, _underlying hash) :
    m_hash(calculate(str))
{
    HASHSTR_ASSERT(m_hash == hash, "Supplied hash does not match what it was calculated as.");
    get_table().insert(m_hash, std::string(str));
}

template<typename _underlying, hash_string_type _type>
basic_hash_string<_underlying, _type>::basic_hash_string(std::string_view str) :
    m_hash(calculate(str))
{
    get_table().insert(m_hash, std::string(str));
}

template<typename _underlying, hash_string_type _type>
basic_hash_string<_underlying, _type>::basic_hash_string(_underlying hash) :
    m_hash(hash)
{ }

template<typename _underlying, hash_string_type _type>
basic_hash_string<_underlying, _type>::~basic_hash_string()
{ }

template<typename _underlying, hash_string_type _type>
_underlying basic_hash_string<_underlying, _type>::get_hash() const
{
    return m_hash;
}

template<typename _underlying, hash_string_type _type>
std::string_view basic_hash_string<_underlying, _type>::try_get_str() const
{
    if( get_table().has(m_hash) )
        return get_table().get_str(m_hash);

    return "";
}

template<typename _underlying, hash_string_type _type>
_underlying basic_hash_string<_underlying, _type>::calculate(std::string_view str)
{
    if constexpr( std::is_same_v<_underlying, u32> )
    {
        static constexpr u32 v0 = 0x7FFFFFFFU;
        return SpookyHash::Hash32(str.data(), str.size(), v0);
    }

    if constexpr( std::is_same_v<_underlying, u64> )
    {
        static constexpr u64 v0 = 0x9ae16a3b2f90404fULL;
        return SpookyHash::Hash64(str.data(), str.size(), v0);
    }

    return -1;
}

template<typename _underlying, hash_string_type _type>
hash_string_table<_underlying>& basic_hash_string<_underlying, _type>::get_table()
{
    static hash_string_table<_underlying> sm_table;
    return sm_table;
}

template<typename _underlying>
void hash_string_table<_underlying>::insert(_underlying hash, std::string&& str)
{
    u64 index = find(hash);
    if( index < m_hashes.size() && m_hashes[index] == hash )
        return;

    m_hashes.insert(index, hash);
    m_strings.insert(index, std::move(str));
}

template<typename _underlying>
void hash_string_table<_underlying>::remove(_underlying hash)
{
    u64 index = find(hash);
    if( index < m_hashes.size() && m_hashes[index] != hash )
        return;

    m_hashes.erase(index);
    m_strings.erase(index);
}

template<typename _underlying>
bool hash_string_table<_underlying>::has(_underlying hash) const
{
    u64 index = find(hash);
    return index < m_hashes.size() && m_hashes[index] == hash;
}

template<typename _underlying>
std::string_view hash_string_table<_underlying>::get_str(_underlying hash) const
{
    u64 index = find(hash);
    if( index >= m_hashes.size() || m_hashes[index] == hash )
        return m_strings[index];

    return "";
}

template<typename _underlying>
u64 hash_string_table<_underlying>::find(_underlying find) const
{
    auto it = std::lower_bound(m_hashes.cbegin(), m_hashes.cend(), find);
    return m_hashes.index_of(it);
}

} // dt