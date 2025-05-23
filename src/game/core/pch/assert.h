﻿#pragma once

#include "system/assert.h"
#include <source_location>

template<typename ...Args>
[[noreturn]]
__forceinline void QuitFmt_Internal(const std::source_location& loc, const char* fmt, Args&&... args) noexcept
{
    // stringstream but we're quitting anyway so fuck it
    std::stringstream ss{ };
    ss << "{} {} ln{}\n" << fmt;
    //SYSLOG_FATAL(ss.str().c_str(), loc.file_name(), loc.function_name(), loc.line(), std::forward<Args>(args)...);
    //SYSLOG_FORCEFLUSH();
    SYSASSERT(false, "Problem..");
    std::abort();
}

#ifdef CFG_DEBUG
#define BREAKFMT(msg, ...) do{ SYSMSG_WARN(msg, __VA_ARGS__); SYSBREAK; }while(0)
#define QUITFMT(msg, ...) do{ QuitFmt_Internal(std::source_location::current(), msg, __VA_ARGS__ ); }while(0)
#else
#define BREAKFMT(msg, ...) do{ SYSMSG_WARN(msg, __VA_ARGS__); }while(0)
#define QUITFMT(msg, ...) do{ QuitFmt_Internal(std::source_location::current(), msg, __VA_ARGS__ ); }while(0)
#endif

#define TRAP_NEQ(left, right, msg, ...) do{ if((left) != (right)){ BREAKFMT(msg, __VA_ARGS__); } }while(0)
#define TRAP_EQ(left, right, msg, ...) do{ if((left) == (right)){ BREAKFMT(msg, __VA_ARGS__); } }while(0)
#define TRAP_GE(left, right, msg, ...) do{ if((left) >= (right)){ BREAKFMT(msg, __VA_ARGS__); } }while(0)
#define TRAP_LE(left, right, msg, ...) do{ if((left) <= (right)){ BREAKFMT(msg, __VA_ARGS__); } }while(0)
#define TRAP_GT(left, right, msg, ...) do{ if((left) > (right)){ BREAKFMT(msg, __VA_ARGS__); } }while(0)
#define TRAP_LT(left, right, msg, ...) do{ if((left) < (right)){ BREAKFMT(msg, __VA_ARGS__); } }while(0)
