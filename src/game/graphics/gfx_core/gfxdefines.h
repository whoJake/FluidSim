#pragma once
#include "system/assert.h"

#ifdef PLATFORM_WINDOWS
    #define GFX_SUPPORTS_GFX
    #define GFX_EXT_SWAPCHAIN
#endif

namespace gfx
{

SYSDECLARE_CHANNEL(graphics);

#define GFX_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(graphics, fmt, __VA_ARGS__)
#define GFX_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(graphics, fmt, __VA_ARGS__)
#define GFX_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(graphics, fmt, __VA_ARGS__)
#define GFX_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(graphics, fmt, __VA_ARGS__)
#define GFX_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(graphics, fmt, __VA_ARGS__)
#define GFX_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(graphics, fmt, __VA_ARGS__)
#define GFX_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(graphics, fmt, __VA_ARGS__)

#define GFX_ASSERT(val, fmt, ...) SYSASSERT(val, SYSMSG_CHANNEL_ASSERT(graphics, fmt, __VA_ARGS__))

}

#define GFX_VIRTUAL_DEVICE 0

#define GFX_DEPENDENCY_NAMES 1

#define GFX_MAX_VERTEX_INPUT_CHANNELS 4
#define GFX_MAX_VERTEX_ATTRIBUTES_PER_CHANNEL 4
#define GFX_MAX_OUTPUT_ATTACHMENTS 8

#define GFX_HAS_IMPL(impl_name) \
template<typename T>\
T get_impl()\
{ return static_cast<T>(impl_name); }\
template<typename T>\
T get_impl() const\
{ return static_cast<T>(impl_name); }

#include "vulkan/vkdefines.h"

// This is fucked.
using surface_create_func = std::function<bool(VkInstance, VkSurfaceKHR*)>;