#pragma once
#include "assert.h"

namespace sys
{

// memory
SYSDECLARE_CHANNEL(memory);
#define MEM_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(memory, fmt, __VA_ARGS__)
#define MEM_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(memory, fmt, __VA_ARGS__)
#define MEM_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(memory, fmt, __VA_ARGS__)
#define MEM_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(memory, fmt, __VA_ARGS__)
#define MEM_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(memory, fmt, __VA_ARGS__)
#define MEM_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(memory, fmt, __VA_ARGS__)
#define MEM_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(memory, fmt, __VA_ARGS__)

#define MEM_ASSERT(cond, fmt, ...) SYSASSERT(cond, SYSMSG_CHANNEL_ASSERT(memory, fmt, __VA_ARGS__))

} // sys