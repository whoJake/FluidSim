#pragma once

#include "system/tracked_allocator.h"
#include "system/assert.h"

#ifndef DT_DEBUG_LEVEL
    #define DT_DEBUG_LEVEL 1
#endif

#ifndef DT_VECTOR_DEBUG_LEVEL
    #define DT_VECTOR_DEBUG_LEVEL DT_DEBUG_LEVEL
#endif

namespace dt
{
SYSDECLARE_CHANNEL(datatypes);
} // dt

#define DTMSG_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(datatypes, fmt, __VA_ARGS__)
#define DTMSG_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(datatypes, fmt, __VA_ARGS__)
#define DTMSG_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(datatypes, fmt, __VA_ARGS__)
#define DTMSG_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(datatypes, fmt, __VA_ARGS__)
#define DTMSG_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(datatypes, fmt, __VA_ARGS__)
#define DTMSG_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(datatypes, fmt, __VA_ARGS__)
#define DTMSG_ASSERT(fmt, ...) SYSMSG_CHANNEL_ASSERT(datatypes, fmt, __VA_ARGS__)
#define DTMSG_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(datatypes, fmt, __VA_ARGS__)

#if DT_DEBUG_LEVEL > 0
    #define DT_ASSERT(cond, fmt, ...) SYSASSERT(cond, DTMSG_FATAL(fmt, __VA_ARGS__))
#else
    #define DT_ASSERT(cond, fmt, ...) SYSASSERT(cond, DTMSG_ASSERT(fmt, __VA_ARGS__))
#endif

namespace dt
{

template<sys::memory_zone zone>
struct zoned_allocator
{
    static constexpr void* allocate(u64 size, u64 align)
    {
        SYSZONE_USE(zone);
        return sys::allocator::get_main()->allocate(size, align);
    }

    static constexpr void free(void* ptr)
    {
        SYSZONE_USE(zone);
        sys::allocator::get_main()->free(ptr);
    }
};

using default_allocator = zoned_allocator<sys::MEMZONE_DEFAULT>;

} // dt