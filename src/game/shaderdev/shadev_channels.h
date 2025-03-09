#pragma once
#include "system/assert.h"

SYSDECLARE_CHANNEL(shaderdev);

#define SHADEV_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(shaderdev, fmt, __VA_ARGS__)

#define SHADEV_ASSERT(val, fmt, ...) SYSASSERT(val, SYSMSG_CHANNEL_ASSERT(shaderdev, fmt, __VA_ARGS__))
