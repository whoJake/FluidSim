#pragma once
#include "gfxdefines.h"

namespace gfx
{

enum memory_zone : u32
{
    MEMZONE_SHADERS = 2,
};

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

    R32_SFLOAT,
    R32G32_SFLOAT,
    R32G32B32_SFLOAT,
    R32G32B32A32_SFLOAT,

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
    case format::R32_SFLOAT:
    case format::A2B10G10R10_UNORM:
        return 4;
    case format::R32G32_SFLOAT:
        return 8;
    case format::R32G32B32_SFLOAT:
        return 12;
    case format::R32G32B32A32_SFLOAT:
        return 16;
    default:
        return 0;
    }
}

constexpr bool has_depth_channel(format format)
{
    switch( format )
    {
    case format::UNDEFINED:
    default:
        return false;
    }
}

enum memory_type : u32
{
    MEMORY_TYPE_CPU_VISIBLE = 0,
    MEMORY_TYPE_GPU_ONLY,
};

enum resource_type : u32
{
    RESOURCE_TYPE_BUFFER = 0,
    RESOURCE_TYPE_TEXTURE,

};

enum resource_view_type : u32
{
    RESOURCE_VIEW_1D = 0,
    RESOURCE_VIEW_2D,
    RESOURCE_VIEW_3D,
    RESOURCE_VIEW_CUBE,

    RESOURCE_VIEW_INHERIT,
};

// Ordered to match VkBufferUsageFlagBits
enum buffer_usage_bits : u32
{
    BUFFER_USAGE_TRANSFER_SRC = 1 << 0,
    BUFFER_USAGE_TRANSFER_DST = 1 << 1,
    BUFFER_USAGE_UNIFORM_TEXEL = 1 << 2,
    BUFFER_USAGE_STORAGE_TEXEL = 1 << 3,
    BUFFER_USAGE_UNIFORM = 1 << 4,
    BUFFER_USAGE_STORAGE = 1 << 5,
    BUFFER_USAGE_INDEX = 1 << 6,
    BUFFER_USAGE_VERTEX = 1 << 7,
    BUFFER_USAGE_INDIRECT = 1 << 8,
};

using buffer_usage_flags = std::underlying_type_t<buffer_usage_bits>;

enum class index_buffer_type : u32
{
    INDEX_TYPE_U16 = 0,
    INDEX_TYPE_U32,
};

enum class vertex_type : u32
{
    VERTEX_TYPE_POSITION = 0,
    VERTEX_TYPE_NORMAL,
    VERTEX_TYPE_TEX0,
    VERTEX_TYPE_TEX1,
    VERTEX_TYPE_TEX2,
    VERTEX_TYPE_TEX3,

    VERTEX_TYPE_COUNT,
};

// Ordered to match VkImageUsageFlagBits
enum texture_usage_bits : u32
{
    TEXTURE_USAGE_TRANSFER_SRC = 1 << 0,
    TEXTURE_USAGE_TRANSFER_DST = 1 << 1,
    TEXTURE_USAGE_SAMPLED = 1 << 2,
    TEXTURE_USAGE_STORAGE = 1 << 3,
    TEXTURE_USAGE_COLOR = 1 << 4,
    TEXTURE_USAGE_DEPTH_STENCIL = 1 << 5,

    TEXTURE_USAGE_SWAPCHAIN_OWNED = 1 << 30,
};

using texture_usage_flags = std::underlying_type_t<texture_usage_bits>;

enum class texture_layout : u32
{
    // Matches VkImageLayout
    TEXTURE_LAYOUT_UNDEFINED = 0,
    TEXTURE_LAYOUT_GENERAL,
    TEXTURE_LAYOUT_COLOR_ATTACHMENT,
    TEXTURE_LAYOUT_DEPTH_STENCIL_ATTACHMENT,
    TEXTURE_LAYOUT_DEPTH_STENCIL_READONLY,
    TEXTURE_LAYOUT_SHADER_READONLY,
    TEXTURE_LAYOUT_TRANSFER_SRC,
    TEXTURE_LAYOUT_TRANSFER_DST,
    TEXTURE_LAYOUT_PREINITIALISED,

    // Does not match VkImageLayout
    TEXTURE_LAYOUT_PRESENT,

    TEXTURE_LAYOUT_COUNT,
};

// Ordered to match VkPresentModeKHR
enum class present_mode
{
    PRESENT_MODE_IMMEDIATE = 0,
    PRESENT_MODE_MAILBOX,
    PRESENT_MODE_FIFO,
    PRESENT_MODE_FIFO_RELAXED,
    
    PRESENT_MODE_COUNT,
};

enum pipeline_stage_flag_bits : u32
{
    PIPELINE_STAGE_TOP_OF_PIPE = 1 << 0,
    PIPELINE_STAGE_DRAW_INDIRECT = 1 << 1,
    PIPELINE_STAGE_VERTEX_INPUT = 1 << 2,
    PIPELINE_STAGE_VERTEX_SHADER = 1 << 3,
    PIPELINE_STAGE_GEOMETRY_SHADER = 1 << 4,
    PIPELINE_STAGE_FRAGMENT_SHADER = 1 << 5,
    PIPELINE_STAGE_EARLY_FRAGMENT_TESTS = 1 << 6,
    PIPELINE_STAGE_LATE_FRAGMENT_TESTS = 1 << 7,
    PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT = 1 << 8,
    PIPELINE_STAGE_TRANSFER = 1 << 9,
    PIPELINE_STAGE_COMPUTE_SHADER = 1 << 10,
    PIPELINE_STAGE_BOTTOM_OF_PIPE = 1 << 11,
};
using pipeline_stage_flags = std::underlying_type_t<pipeline_stage_flag_bits>;

enum shader_stage_flag_bits : u32
{
    SHADER_STAGE_VERTEX = 1 << 0,
    SHADER_STAGE_GEOMETRY = 1 << 1,
    SHADER_STAGE_FRAGMENT = 1 << 2,
    SHADER_STAGE_COMPUTE = 1 << 3,

    SHADER_STAGE_ALL =
        SHADER_STAGE_VERTEX ||
        SHADER_STAGE_GEOMETRY ||
        SHADER_STAGE_FRAGMENT ||
        SHADER_STAGE_COMPUTE,

    SHADER_STAGE_FINAL = SHADER_STAGE_COMPUTE + 1,
};
using shader_stage_flags = std::underlying_type_t<shader_stage_flag_bits>;

enum shader_resource_type : u32
{
    SHADER_RESOURCE_INPUT = 0,
    SHADER_RESOURCE_INPUT_ATTACHMENT,
    SHADER_RESOURCE_OUTPUT,
    SHADER_RESOURCE_IMAGE,
    SHADER_RESOURCE_IMAGE_SAMPLER,
    SHADER_RESOURCE_IMAGE_STORAGE,
    SHADER_RESOURCE_SAMPLER,
    SHADER_RESOURCE_UNIFORM_BUFFER,
    SHADER_RESOURCE_UNIFORM_BUFFER_DYNAMIC,
    SHADER_RESOURCE_STORAGE_BUFFER,
    SHADER_RESOURCE_STORAGE_BUFFER_DYNAMIC,
    SHADER_RESOURCE_PUSH_CONSTANT,
    SHADER_RESOURCE_SPECIALIZATION_CONSTANT,
    SHADER_RESOURCE_EMPTY,

    SHADER_RESOURCE_COUNT,
};

enum descriptor_table_type : u32
{
    DESCRIPTOR_TABLE_PER_FRAME = 0,
    DESCRIPTOR_TABLE_PER_VIEWPORT,
    DESCRIPTOR_TABLE_PER_PASS,
    DESCRIPTOR_TABLE_PER_DRAW,

    DESCRIPTOR_TABLE_COUNT,
};

enum swapchain_acquire_result : u32
{
    SWAPCHAIN_ACQUIRE_SUCCESS = 0,
    SWAPCHAIN_ACQUIRE_SUBOPTIMAL,
    SWAPCHAIN_ACQUIRE_OUT_OF_DATE,
};

/// <summary>
/// A dynamic shader resource means that a dynamic offset can be applied
/// to the buffer when binding it to the command list.
/// </summary>
constexpr bool is_dynamic_shader_resource_type(shader_resource_type type)
{
    switch( type )
    {
    case shader_resource_type::SHADER_RESOURCE_UNIFORM_BUFFER_DYNAMIC:
    case shader_resource_type::SHADER_RESOURCE_STORAGE_BUFFER_DYNAMIC:
        return true;
    default:
        return false;
    }
}

struct shader_pass_outputs
{
    // u32 viewMask; ??
    u32 color_output_count;
    format color_outputs[GFX_MAX_OUTPUT_ATTACHMENTS];
    format depth_output{ format::UNDEFINED };
    format stencil_output{ format::UNDEFINED };
};

} // gfx