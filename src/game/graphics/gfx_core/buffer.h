#pragma once

#include "allocator.h"

namespace gfx
{

// Ordered to match VkBufferUsageFlagBits
enum buffer_usage_bits : u32
{
    transfer_src = 1 << 0,
    transfer_dst = 1 << 1,
    uniform_texel = 1 << 2,
    storage_texel = 1 << 3,
    index_buffer = 1 << 4,
    vertex_buffer = 1 << 5,
    indirect_buffer = 1 << 6,
};

using buffer_usage = std::underlying_type_t<buffer_usage_bits>;

class buffer
{
public:
    buffer(allocation_info allocation, buffer_usage usage, void* pImpl);
    ~buffer() = default;

    DEFAULT_MOVE(buffer);
    DEFAULT_COPY(buffer);

    allocation_info& get_allocation();

    buffer_usage get_usage() const;

    void* get_impl_ptr();
private:
    allocation_info m_allocation;
    void* m_impl;
    buffer_usage m_usage;
};

} // gfx