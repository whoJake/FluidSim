#pragma once

#ifndef NOMINMAX
    #define NOMINMAX
#endif // NOMINMAX

#include <type_traits>
#include <limits>

using u8 = uint8_t;
#define u8_max std::numeric_limits<u8>::max()
#define u8_min std::numeric_limits<u8>::min()
#define u8_cast(v) static_cast<u8>(v)
using i8 = int8_t;
#define i8_max std::numeric_limits<i8>::max()
#define i8_min std::numeric_limits<i8>::min()
#define i8_cast(v) static_cast<i8>(v)

using u16 = uint16_t;
#define u16_max std::numeric_limits<u16>::max()
#define u16_min std::numeric_limits<u16>::min()
#define u16_cast(v) static_cast<u16>(v)
using i16 = int16_t;
#define i16_max std::numeric_limits<i16>::max()
#define i16_min std::numeric_limits<i16>::min()
#define i16_cast(v) static_cast<i16>(v)

using u32 = uint32_t;
#define u32_max std::numeric_limits<u32>::max()
#define u32_min std::numeric_limits<u32>::min()
#define u32_cast(v) static_cast<u32>(v)
using i32 = int32_t;
#define i32_max std::numeric_limits<i32>::max()
#define i32_min std::numeric_limits<i32>::min()
#define i32_cast(v) static_cast<i32>(v)

using u64 = uint64_t;
#define u64_max std::numeric_limits<u64>::max()
#define u64_min std::numeric_limits<u64>::min()
#define u64_cast(v) static_cast<u64>(v)
using i64 = int64_t;
#define i64_max std::numeric_limits<i64>::max()
#define i64_min std::numeric_limits<i64>::min()
#define i64_cast(v) static_cast<i64>(v)

using f32 = float;
#define f32_max std::numeric_limits<f32>::max()
#define f32_min -f32_max
#define f32_cast(v) static_cast<f32>(v)

using f64 = double;
#define f64_max std::numeric_limits<f64>::max()
#define f64_min -f64_max
#define f64_cast(v) static_cast<f64>(v)