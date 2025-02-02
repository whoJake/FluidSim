#pragma once
#include "vector.h"

namespace dt
{

using default_bitset_allocator = zoned_allocator<sys::MEMZONE_DEFAULT>;

template<typename _underlying>
class bitset : protected vector<_underlying>
{
public:
    static constexpr u64 bits_per_underlying = sizeof(_underlying) * 8;

    bitset(allocator* alloc_method = default_bitset_allocator::get());
    bitset(u64 initial_size, allocator* alloc_method = default_bitset_allocator::get());

    bool is_set(u64 bit) const;
    void set(u64 bit, bool value);

    void resize(u64 bits);
private:
    using _base = vector<_underlying>;

    u64 bit_to_item_index(u64 bit) const;
    _underlying bit_mask(u64 bit) const;

    u64 size_required(u64 bit) const;
    void set_size(u64 bits);
};

template<typename _underlying, sys::memory_zone zone>
class zoned_bitset : public bitset<_underlying>
{
public:
    zoned_bitset() :
        bitset<_underlying>(zoned_allocator<zone>::get())
    { }

    zoned_bitset(u64 initial_size) :
        bitset<_underlying>(initial_size, zoned_allocator<zone>::get())
    { }
};

using u8_bitset = bitset<u8>;
using u16_bitset = bitset<u16>;
using u32_bitset = bitset<u32>;
using u64_bitset = bitset<u64>;

template<sys::memory_zone zone>
using u8_zoned_bitset = zoned_bitset<u8, zone>;

template<sys::memory_zone zone>
using u16_zoned_bitset = zoned_bitset<u16, zone>;

template<sys::memory_zone zone>
using u32_zoned_bitset = zoned_bitset<u32, zone>;

template<sys::memory_zone zone>
using u64_zoned_bitset = zoned_bitset<u64, zone>;

} // dt

#ifndef INC_DT_BITSET_INL
    #define INC_DT_BITSET_INL
    #include "bitset.inl"
#endif