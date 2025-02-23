#pragma once
#include "hashing/spookyhash.h"

namespace sys
{

inline void combine_hash64_value(u64& in_out, u64 combine)
{
    // Taken from glm::detail::hash_combine
    combine += 0x9e3779b9 + (in_out << 6) + (in_out >> 2);
    in_out ^= combine;
}

template<class T>
inline void hash_combine(u64& seed, const T& b)
{
    std::hash<T> hasher;
    u64 hash = hasher(b);

    combine_hash64_value(seed, hash);
}

inline u32 hash32(const void* data, u64 size)
{
    return SpookyHash::Hash32(data, size, 0x7FFFFFFFU);
}

inline u64 hash64(const void* data, u64 size)
{
    return SpookyHash::Hash64(data, size, 0x9ae16a3b2f90404fULL);
}

} // sys