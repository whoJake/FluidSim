#pragma once

namespace cdt
{

enum class image_format
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

constexpr u32 get_channel_count(image_format format)
{
    switch( format )
    {
    case image_format::R8_UNORM:
    case image_format::R8_SNORM:
    case image_format::R8_UINT:
    case image_format::R8_SINT:
    case image_format::R8_SRGB:
        return 1;
    case image_format::R8G8_UNORM:
    case image_format::R8G8_SNORM:
    case image_format::R8G8_UINT:
    case image_format::R8G8_SINT:
    case image_format::R8G8_SRGB:
        return 2;
    case image_format::R8G8B8_UNORM:
    case image_format::R8G8B8_SNORM:
    case image_format::R8G8B8_UINT:
    case image_format::R8G8B8_SINT:
    case image_format::R8G8B8_SRGB:
        return 3;
    case image_format::R8G8B8A8_UNORM:
    case image_format::R8G8B8A8_SNORM:
    case image_format::R8G8B8A8_UINT:
    case image_format::R8G8B8A8_SINT:
    case image_format::R8G8B8A8_SRGB:
    case image_format::A2B10G10R10_UNORM:
        return 4;
    default:
        return 0;
    }
}

constexpr bool is_hdr_format(image_format format)
{
    switch( format )
    {
    case image_format::A2B10G10R10_UNORM:
        return true;
    default:
        return false;
    }
}

constexpr bool is_depth_format(image_format format)
{
    // TODO
    return false;
}

constexpr u32 get_bits_per_pixel(image_format format)
{
    switch( format )
    {
    case image_format::R8_UNORM:
    case image_format::R8_SNORM:
    case image_format::R8_UINT:
    case image_format::R8_SINT:
    case image_format::R8_SRGB:
        return 8;
    case image_format::R8G8_UNORM:
    case image_format::R8G8_SNORM:
    case image_format::R8G8_UINT:
    case image_format::R8G8_SINT:
    case image_format::R8G8_SRGB:
        return 16;
    case image_format::R8G8B8_UNORM:
    case image_format::R8G8B8_SNORM:
    case image_format::R8G8B8_UINT:
    case image_format::R8G8B8_SINT:
    case image_format::R8G8B8_SRGB:
        return 24;
    case image_format::R8G8B8A8_UNORM:
    case image_format::R8G8B8A8_SNORM:
    case image_format::R8G8B8A8_UINT:
    case image_format::R8G8B8A8_SINT:
    case image_format::R8G8B8A8_SRGB:
    case image_format::A2B10G10R10_UNORM:
        return 32;
    default:
        return 0;
    }
}

constexpr bool has_alpha(image_format format)
{
    switch( format )
    {
    case image_format::R8G8B8A8_UNORM:
    case image_format::R8G8B8A8_SNORM:
    case image_format::R8G8B8A8_UINT:
    case image_format::R8G8B8A8_SINT:
    case image_format::R8G8B8A8_SRGB:
    case image_format::A2B10G10R10_UNORM:
        return true;
    default:
        return false;
    }
}

} // cdt