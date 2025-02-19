#pragma once
#include "system/magic_numbers.h"
#include "../types.h"

namespace gfx
{

/// -----------------------------------------------------------------------------------------------------------------------------
/// | MAGIC | compiled_header | compiled_shader_desc N | shader_pass_compiled N | program_name | | entry_point | spirv_data | N |
/// -----------------------------------------------------------------------------------------------------------------------------

constexpr u32 PROGRAM_MAGIC_NUMBER = MAKE_MAGIC_NUMBER('F', 'X', 'C', 'P');

struct compiled_program
{
    u8 name_size;
    u8 shader_count;
    u8 pass_count;
    u8 pad{ };
};

struct compiled_shader
{
    u32 entry_point_offset;
    u8 entry_point_size;
    u8 pad[3]{ };
    shader_stage_flag_bits stage;
    u32 data_size_32;
};

struct compiled_pass
{
    shader_stage_flags stage_mask;
    u8 vertex_index;
    u8 geometry_index;
    u8 fragment_index;
    u8 compute_index;
};

} // gfx