#pragma once
#include "gfx_core/types.h"
#include "data/program_desc.h"
#include "gfx_core/loaders/shader_loader.h"


bool compile_program(const shader_program_file& program_file, gfx::program_def& compiled_program);

/// Compiler Shader : Compiles a given shader by filename and returns the compiled code
/// in the out_data pointer. Returns whether the compilation was successful or not.
bool compile_shader(const char* filename, const char* entry_point, gfx::shader_stage_flag_bits stage, std::vector<u32>* out_data);



