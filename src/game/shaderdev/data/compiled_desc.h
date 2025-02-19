#pragma once
#include "gfx_core/types.h"
#include "system/magic_numbers.h"

struct shader_compiled
{
    std::string entry_point;
    gfx::shader_stage_flag_bits stage;
    std::vector<u32> data;
};

struct shader_pass_compiled
{
    gfx::shader_stage_flags stage_mask;

    u8 vertex_index;
    u8 geometry_index;
    u8 fragment_index;
    u8 compute_index;
    
    // TODO reflected tables
};

struct shader_program_compiled
{
    std::string name;
    std::vector<shader_pass_compiled> passes;
    std::vector<shader_compiled> shaders;
};