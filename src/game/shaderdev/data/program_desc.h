#pragma once
#include "gfx_core/types.h"

struct shader_file_desc
{
    std::string filepath;
    std::string entry_point;
    gfx::shader_stage_flag_bits stage;
};

struct shader_pass_desc
{
    static constexpr u32 invalid_index = u32_max;

    u32 vertex_index{ invalid_index };
    u32 geometry_index{ invalid_index };
    u32 fragment_index{ invalid_index };
    u32 compute_index{ invalid_index };
};

struct shader_program_file
{
    std::string name;
    std::vector<shader_pass_desc> passes;
    std::vector<shader_file_desc> shader_files;
};