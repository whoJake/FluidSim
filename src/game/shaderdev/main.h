#pragma once
#include "gfx_core/types.h"
#include "system/assert.h"

#include "data/program_desc.h"
#include "data/compiled_desc.h"

SYSDECLARE_CHANNEL(shaderdev);

#define SHADEV_VERBOSE(fmt, ...) SYSMSG_CHANNEL_VERBOSE(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_PROFILE(fmt, ...) SYSMSG_CHANNEL_PROFILE(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_DEBUG(fmt, ...) SYSMSG_CHANNEL_DEBUG(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_INFO(fmt, ...) SYSMSG_CHANNEL_INFO(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_WARN(fmt, ...) SYSMSG_CHANNEL_WARN(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_ERROR(fmt, ...) SYSMSG_CHANNEL_ERROR(shaderdev, fmt, __VA_ARGS__)
#define SHADEV_FATAL(fmt, ...) SYSMSG_CHANNEL_FATAL(shaderdev, fmt, __VA_ARGS__)

#define SHADEV_ASSERT(val, fmt, ...) SYSASSERT(val, SYSMSG_CHANNEL_ASSERT(shaderdev, fmt, __VA_ARGS__))

bool parse_program(const char* filename, shader_program_file& program);

bool compile_program(const shader_program_file& program_file, shader_program_compiled& compiled_program);

/// Compiler Shader : Compiles a given shader by filename and returns the compiled code
/// in the out_data pointer. Returns whether the compilation was successful or not.
bool compile_shader(const char* filename, const char* entry_point, gfx::shader_stage_flag_bits stage, std::vector<u32>* out_data);

bool write_program(const char* filename, const shader_program_compiled& compiled_program);



