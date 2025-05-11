#include "bitset.h"

namespace dt
{

template<typename _underlying, typename _allocator>
bitset<_underlying, _allocator>::bitset() :
    _base(1)
{
    set_size(1);
}

template<typename _underlying, typename _allocator>
bitset<_underlying, _allocator>::bitset(u64 initial_size) :
    _base(bit_to_item_index(initial_size) + 1)
{
    set_size(initial_size);
}

template<typename _underlying, typename _allocator>
bool bitset<_underlying, _allocator>::is_set(u64 bit) const
{
    return _base::at(bit_to_item_index(bit)) & bit_mask(bit);
}

template<typename _underlying, typename _allocator>
void bitset<_underlying, _allocator>::set(u64 bit, bool value)
{
    set_size(bit);

    if( value )
        _base::at(bit_to_item_index(bit)) |= bit_mask(bit);
    else
        _base::at(bit_to_item_index(bit)) &= ~bit_mask(bit);
}

template<typename _underlying, typename _allocator>
void bitset<_underlying, _allocator>::resize(u64 bits)
{
    set_size(bits);
}

template<typename _underlying, typename _allocator>
u64 bitset<_underlying, _allocator>::bit_to_item_index(u64 bit) const
{
    return bit / bits_per_underlying;
}

template<typename _underlying, typename _allocator>
_underlying bitset<_underlying, _allocator>::bit_mask(u64 bit) const
{
    u64 subindex = bit - (bit_to_item_index(bit) * bits_per_underlying);
    return static_cast<_underlying>(subindex);
}

template<typename _underlying, typename _allocator>
u64 bitset<_underlying, _allocator>::size_required(u64 bits) const
{
    return bit_to_item_index(bits) + 1;
}

template<typename _underlying, typename _allocator>
void bitset<_underlying, _allocator>::set_size(u64 bits)
{
    if( size_required(bits) == _base::size() )
        return;

    _base::resize(size_required(bits), 0);
}

} // dt