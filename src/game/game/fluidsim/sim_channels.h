#pragma once
#include "system/assert.h"

// fluidsim
SYSDECLARE_CHANNEL(fluidsim);
#define FLUIDSIM_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(fluidsim, fmt, __VA_ARGS__)
#define FLUIDSIM_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(fluidsim, fmt, __VA_ARGS__)
#define FLUIDSIM_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(fluidsim, fmt, __VA_ARGS__)
#define FLUIDSIM_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(fluidsim, fmt, __VA_ARGS__)
#define FLUIDSIM_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(fluidsim, fmt, __VA_ARGS__)
#define FLUIDSIM_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(fluidsim, fmt, __VA_ARGS__)
#define FLUIDSIM_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(fluidsim, fmt, __VA_ARGS__)

#define FLUIDSIM_ASSERT(cond, fmt, ...) SYSASSERT(cond, SYSMSG_CHANNEL_ASSERT(fluidsim, fmt, __VA_ARGS__))