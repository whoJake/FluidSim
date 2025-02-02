#include "bitset.h"

namespace dt
{

template<typename _underlying>
bitset<_underlying>::bitset(allocator* alloc_method) :
    _base(1, alloc_method)
{
    set_size(1);
}

template<typename _underlying>
bitset<_underlying>::bitset(u64 initial_size, allocator* alloc_method) :
    _base(bit_to_item_index(initial_size) + 1, alloc_method)
{
    set_size(initial_size);
}

template<typename _underlying>
bool bitset<_underlying>::is_set(u64 bit) const
{
    return _base::at(bit_to_item_index(bit)) & bit_mask(bit);
}

template<typename _underlying>
void bitset<_underlying>::set(u64 bit, bool value)
{
    set_size(bit);

    if( value )
        _base::at(bit_to_item_index(bit)) |= bit_mask(bit);
    else
        _base::at(bit_to_item_index(bit)) &= ~bit_mask(bit);
}

template<typename _underlying>
void bitset<_underlying>::resize(u64 bits)
{
    set_size(bits);
}

template<typename _underlying>
u64 bitset<_underlying>::bit_to_item_index(u64 bit) const
{
    return (bit + 1) / bits_per_underlying;
}

template<typename _underlying>
_underlying bitset<_underlying>::bit_mask(u64 bit) const
{
    u64 subindex = bit - (bit_to_item_index(bit) * bits_per_underlying);
    return 1 << static_cast<_underlying>(subindex);
}

template<typename _underlying>
u64 bitset<_underlying>::size_required(u64 bits) const
{
    return bit_to_item_index(bits) + 1;
}

template<typename _underlying>
void bitset<_underlying>::set_size(u64 bits)
{
    if( size_required(bits) == _base::size() )
        return;

    _base::resize(size_required(bits), 0);
}

} // dt