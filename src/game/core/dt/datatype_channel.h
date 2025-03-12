#pragma once
#include "system/assert.h"

namespace dt
{

SYSDECLARE_CHANNEL(datatypes);
#define DT_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(datatypes, fmt, __VA_ARGS__)
#define DT_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(datatypes, fmt, __VA_ARGS__)
#define DT_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(datatypes, fmt, __VA_ARGS__)
#define DT_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(datatypes, fmt, __VA_ARGS__)
#define DT_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(datatypes, fmt, __VA_ARGS__)
#define DT_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(datatypes, fmt, __VA_ARGS__)
#define DT_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(datatypes, fmt, __VA_ARGS__)

#define DT_ASSERT(cond, fmt, ...) SYSASSERT(cond, SYSMSG_CHANNEL_ASSERT(datatypes, fmt, __VA_ARGS__))

} // dt