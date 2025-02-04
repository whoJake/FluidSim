#pragma once
#include "vector.h"

namespace dt
{

template<typename _underlying, typename _allocator = default_allocator>
class bitset : protected vector<_underlying, _allocator>
{
public:
    static constexpr u64 bits_per_underlying = sizeof(_underlying) * 8;

    bitset();
    bitset(u64 initial_size);

    bool is_set(u64 bit) const;
    void set(u64 bit, bool value);

    void resize(u64 bits);
private:
    using _base = vector<_underlying, _allocator>;

    u64 bit_to_item_index(u64 bit) const;
    _underlying bit_mask(u64 bit) const;

    u64 size_required(u64 bit) const;
    void set_size(u64 bits);
};

using u8_bitset = bitset<u8>;
using u16_bitset = bitset<u16>;
using u32_bitset = bitset<u32>;
using u64_bitset = bitset<u64>;

} // dt

#ifndef INC_DT_BITSET_INL
    #define INC_DT_BITSET_INL
    #include "bitset.inl"
#endif