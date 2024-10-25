#pragma once

#include "resource.h"
#include "memory.h"

namespace gfx
{

// Ordered to match VkBufferUsageFlagBits
enum buffer_usage_bits : u32
{
    buffer_transfer_src = 1 << 0,
    buffer_transfer_dst = 1 << 1,
    uniform_texel = 1 << 2,
    storage_texel = 1 << 3,
    index_buffer = 1 << 4,
    vertex_buffer = 1 << 5,
    indirect_buffer = 1 << 6,
};

using buffer_usage = std::underlying_type_t<buffer_usage_bits>;

class buffer : public resource
{
public:
    buffer(memory_info allocation, buffer_usage usage, void* pImpl);
    ~buffer() = default;

    DEFAULT_MOVE(buffer);
    DEFAULT_COPY(buffer);

    buffer_usage get_usage() const;

    template<typename T>
    T get_impl()
    {
        return static_cast<T>(m_impl);
    }
private:
    void* m_impl;
    buffer_usage m_usage;
    u32 m_pad;
};

} // gfx