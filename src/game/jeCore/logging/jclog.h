#pragma once

#include "Log.h"
#include "ConsoleTarget.h"
#include "FileTarget.h"
#include "EmptyTarget.h"

#define JCLOG_NONE(logref, fmt, ...) (logref).none(fmt, __VA_ARGS__)
#define JCLOG_TRACE(logref, fmt, ...) (logref).trace(fmt, __VA_ARGS__)
#define JCLOG_TRACK(logref, fmt, ...) (logref).track(fmt, __VA_ARGS__)
#define JCLOG_DEBUG(logref, fmt, ...) (logref).debug(fmt, __VA_ARGS__)
#define JCLOG_PROFILE(logref, fmt, ...) (logref).profile(fmt, __VA_ARGS__)
#define JCLOG_INFO(logref, fmt, ...) (logref).info(fmt, __VA_ARGS__)
#define JCLOG_WARN(logref, fmt, ...) (logref).warn(fmt, __VA_ARGS__)
#define JCLOG_ERROR(logref, fmt, ...) (logref).error(fmt, __VA_ARGS__)
#define JCLOG_EXCEPTION(logref, ex, fmt, ...) (logref).exception(ex, fmt, __VA_ARGS__)
#define JCLOG_EVENT(logref, fmt, ...) (logref).event(fmt, __VA_ARGS__)
#define JCLOG_LOG(logref, level, fmt, ...) (logref).log(level, fmt, __VA_ARGS__)