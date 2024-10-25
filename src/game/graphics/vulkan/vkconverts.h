#pragma once
#include "vkdefines.h"
#include "cdt/imageformats.h"
#include "gfx_core/memory.h"

namespace gfx
{
namespace converters
{

constexpr VkFormat get_format_vk(cdt::image_format format)
{
    switch( format )
    {
    case cdt::image_format::R8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case cdt::image_format::R8_SNORM:
        return VK_FORMAT_R8_SNORM;
    case cdt::image_format::R8_UINT:
        return VK_FORMAT_R8_UINT;
    case cdt::image_format::R8_SINT:
        return VK_FORMAT_R8_SINT;
    case cdt::image_format::R8_SRGB:
        return VK_FORMAT_R8_SRGB;
    case cdt::image_format::R8G8_UNORM:
        return VK_FORMAT_R8G8_UNORM;
    case cdt::image_format::R8G8_SNORM:
        return VK_FORMAT_R8G8_SNORM;
    case cdt::image_format::R8G8_UINT:
        return VK_FORMAT_R8G8_UINT;
    case cdt::image_format::R8G8_SINT:
        return VK_FORMAT_R8G8_SINT;
    case cdt::image_format::R8G8_SRGB:
        return VK_FORMAT_R8G8_SRGB;
    case cdt::image_format::R8G8B8_UNORM:
        return VK_FORMAT_R8G8B8_UNORM;
    case cdt::image_format::R8G8B8_SNORM:
        return VK_FORMAT_R8G8B8_SNORM;
    case cdt::image_format::R8G8B8_UINT:
        return VK_FORMAT_R8G8B8_UINT;
    case cdt::image_format::R8G8B8_SINT:
        return VK_FORMAT_R8G8B8_SINT;
    case cdt::image_format::R8G8B8_SRGB:
        return VK_FORMAT_R8G8B8_SRGB;
    case cdt::image_format::R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case cdt::image_format::R8G8B8A8_SNORM:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case cdt::image_format::R8G8B8A8_UINT:
        return VK_FORMAT_R8G8B8A8_UINT;
    case cdt::image_format::R8G8B8A8_SINT:
        return VK_FORMAT_R8G8B8A8_SINT;
    case cdt::image_format::R8G8B8A8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case cdt::image_format::A2B10G10R10_UNORM:
        return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}

constexpr VkImageViewType get_view_type_vk(resource_view_type type, bool isArray = false)
{
    switch( type )
    {
    case resource_view_type::texture_1d:
        return isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
    case resource_view_type::texture_2d:
        return isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    case resource_view_type::texture_3d:
        return VK_IMAGE_VIEW_TYPE_3D;
    case resource_view_type::cube:
        return isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
    default:
        return VK_IMAGE_VIEW_TYPE_2D;
    }
}

} // converters
} // gfx