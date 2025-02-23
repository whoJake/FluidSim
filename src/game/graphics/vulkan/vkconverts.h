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

constexpr cdt::image_format get_format_vk_cdt(VkFormat format)
{
    switch( format )
    {
    case VK_FORMAT_R8_UNORM:
        return cdt::image_format::R8_UNORM;
    case VK_FORMAT_R8_SNORM:
        return cdt::image_format::R8_SNORM;
    case VK_FORMAT_R8_UINT:
        return cdt::image_format::R8_UINT;
    case VK_FORMAT_R8_SINT:
        return cdt::image_format::R8_SINT;
    case VK_FORMAT_R8_SRGB:
        return cdt::image_format::R8_SRGB;
    case VK_FORMAT_R8G8_UNORM:
        return cdt::image_format::R8G8_UNORM;
    case VK_FORMAT_R8G8_SNORM:
        return cdt::image_format::R8G8_SNORM;
    case VK_FORMAT_R8G8_UINT:
        return cdt::image_format::R8G8_UINT;
    case VK_FORMAT_R8G8_SINT:
        return cdt::image_format::R8G8_SINT;
    case VK_FORMAT_R8G8_SRGB:
        return cdt::image_format::R8G8_SRGB;
    case VK_FORMAT_R8G8B8_UNORM:
        return cdt::image_format::R8G8B8_SNORM;
    case VK_FORMAT_R8G8B8_SNORM:
        return cdt::image_format::R8G8B8_UNORM;
    case VK_FORMAT_R8G8B8_UINT:
        return cdt::image_format::R8G8B8_UINT;
    case VK_FORMAT_R8G8B8_SINT:
        return cdt::image_format::R8G8B8_SINT;
    case VK_FORMAT_R8G8B8_SRGB:
        return cdt::image_format::R8G8B8_SRGB;
    case VK_FORMAT_R8G8B8A8_UNORM:
        return cdt::image_format::R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SNORM:
        return cdt::image_format::R8G8B8A8_SNORM;
    case VK_FORMAT_R8G8B8A8_UINT:
        return cdt::image_format::R8G8B8A8_UINT;
    case VK_FORMAT_R8G8B8A8_SINT:
        return cdt::image_format::R8G8B8A8_SINT;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return cdt::image_format::R8G8B8A8_SRGB;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        return cdt::image_format::A2B10G10R10_UNORM;
    case VK_FORMAT_UNDEFINED:
    default:
        return cdt::image_format::UNDEFINED;
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

constexpr VkImageLayout get_layout_vk(texture_layout layout)
{
    switch( layout )
    {
    case texture_layout::TEXTURE_LAYOUT_PRESENT:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:
        return (VkImageLayout)layout;
    }
}

constexpr texture_layout get_layout_vk(VkImageLayout layout)
{
    switch( layout )
    {
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        return texture_layout::TEXTURE_LAYOUT_PRESENT;
    default:
        return (texture_layout)layout;
    }
}

constexpr VkShaderStageFlags get_shader_stage_flags_vk(shader_stage_flags flags)
{
    VkShaderStageFlags retval{ };

    if( flags & SHADER_STAGE_VERTEX )
        retval |= VK_SHADER_STAGE_VERTEX_BIT;
    if( flags & SHADER_STAGE_GEOMETRY )
        retval |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if( flags & SHADER_STAGE_FRAGMENT )
        retval |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if( flags & SHADER_STAGE_COMPUTE )
        retval |= VK_SHADER_STAGE_COMPUTE_BIT;

    return retval;
}

constexpr VkDescriptorType get_descriptor_type_vk(shader_resource_type type)
{
    switch( type )
    {
        case SHADER_RESOURCE_INPUT_ATTACHMENT:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case SHADER_RESOURCE_IMAGE:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case SHADER_RESOURCE_IMAGE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case SHADER_RESOURCE_IMAGE_STORAGE:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case SHADER_RESOURCE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case SHADER_RESOURCE_UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case SHADER_RESOURCE_UNIFORM_BUFFER_DYNAMIC:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case SHADER_RESOURCE_STORAGE_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case SHADER_RESOURCE_STORAGE_BUFFER_DYNAMIC:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        default:
            GFX_ASSERT(false, "Shader resource type is not valid for VkDescriptorType conversion.");
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

} // converters
} // gfx