#pragma once
#ifndef NOMINMAX
    #define NOMINMAX
#endif // NOMINMAX

#ifdef PLATFORM_WINDOWS
    #define GFX_SUPPORTS_GFX
#endif

#ifdef GFX_OPTIMIZE
    #define GFX_OPTIMIZATION
#else
    #define GFX_OPTIMIZATION
#endif

#define GFX_MSG(fmt, ...) CHANNEL_LOG_MSG(::sys::log::channel::graphics, fmt, __VA_ARGS__)
#define GFX_VERBOSE(fmt, ...) CHANNEL_LOG_VERBOSE(::sys::log::channel::graphics, fmt, __VA_ARGS__)
#define GFX_PROFILE(fmt, ...) CHANNEL_LOG_PROFILE(::sys::log::channel::graphics, fmt, __VA_ARGS__)
#define GFX_DEBUG(fmt, ...) CHANNEL_LOG_DEBUG(::sys::log::channel::graphics, fmt, __VA_ARGS__)
#define GFX_INFO(fmt, ...) CHANNEL_LOG_INFO(::sys::log::channel::graphics, fmt, __VA_ARGS__)
#define GFX_WARN(fmt, ...) CHANNEL_LOG_WARN(::sys::log::channel::graphics, fmt, __VA_ARGS__)
#define GFX_ERROR(fmt, ...) CHANNEL_LOG_ERROR(::sys::log::channel::graphics, fmt, __VA_ARGS__)
#define GFX_FATAL(fmt, ...) CHANNEL_LOG_FATAL(::sys::log::channel::graphics, fmt, __VA_ARGS__)

#define GFX_ASSERT(val, fmt, ...) if(!(!!(val))){ QUITFMT(fmt, __VA_ARGS__); }

#define GFX_NUM_FRAMES_AHEAD 1
#define GFX_NUM_ACTIVE_FRAMES GFX_NUM_FRAMES_AHEAD + 1

#include "vulkan/vkdefines.h"