#pragma once

namespace gfx
{

enum class format
{
    UNDEFINED = 0,
    R8_UNORM,
    R8_SNORM,
    R8_UINT,
    R8_SINT,
    R8_SRGB,
    R8G8_UNORM,
    R8G8_SNORM,
    R8G8_UINT,
    R8G8_SINT,
    R8G8_SRGB,
    R8G8B8_UNORM,
    R8G8B8_SNORM,
    R8G8B8_UINT,
    R8G8B8_SINT,
    R8G8B8_SRGB,
    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R8G8B8A8_UINT,
    R8G8B8A8_SINT,
    R8G8B8A8_SRGB,

    A2B10G10R10_UNORM,
};

constexpr u32 get_format_stride(format format)
{
    switch( format )
    {
    case format::R8_UNORM:
    case format::R8_SNORM:
    case format::R8_UINT:
    case format::R8_SINT:
    case format::R8_SRGB:
        return 1;
    case format::R8G8_UNORM:
    case format::R8G8_SNORM:
    case format::R8G8_UINT:
    case format::R8G8_SINT:
    case format::R8G8_SRGB:
        return 2;
    case format::R8G8B8_UNORM:
    case format::R8G8B8_SNORM:
    case format::R8G8B8_UINT:
    case format::R8G8B8_SINT:
    case format::R8G8B8_SRGB:
        return 3;
    case format::R8G8B8A8_UNORM:
    case format::R8G8B8A8_SNORM:
    case format::R8G8B8A8_UINT:
    case format::R8G8B8A8_SINT:
    case format::R8G8B8A8_SRGB:
    case format::A2B10G10R10_UNORM:
        return 4;
    default:
        return 0;
    }
}

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

enum class index_buffer_type : u32
{
    u16_type = 0,
    u32_type,
};

enum class vertex_type : u32
{
    position = 0,
    normal,
    tex0,
    tex1,
    tex2,
    tex3,

    count,
};

} // gfx