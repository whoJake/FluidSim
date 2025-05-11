#pragma once
#include "system/assert.h"

SYSDECLARE_CHANNEL(compiler);

#define COMPILER_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(compiler, fmt, __VA_ARGS__)
#define COMPILER_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(compiler, fmt, __VA_ARGS__)
#define COMPILER_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(compiler, fmt, __VA_ARGS__)
#define COMPILER_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(compiler, fmt, __VA_ARGS__)
#define COMPILER_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(compiler, fmt, __VA_ARGS__)
#define COMPILER_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(compiler, fmt, __VA_ARGS__)
#define COMPILER_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(compiler, fmt, __VA_ARGS__)

#define COMPILER_ASSERT(val, fmt, ...) SYSASSERT(val, SYSMSG_CHANNEL_ASSERT(compiler, fmt, __VA_ARGS__))