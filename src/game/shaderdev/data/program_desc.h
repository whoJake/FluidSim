#pragma once
#include "gfx_core/types.h"
#include "gfx_core/pipeline_state.h"

struct shader_file_desc
{
    std::string filepath;
    std::string entry_point;
    gfx::shader_stage_flag_bits stage;
};

struct color_output_desc
{
    gfx::format format;
    gfx::color_blend_state blend_state;
};

struct shader_pass_desc
{
    static constexpr u32 invalid_index = u32_max;

    u32 vertex_index{ invalid_index };
    u32 geometry_index{ invalid_index };
    u32 fragment_index{ invalid_index };
    u32 compute_index{ invalid_index };

    gfx::pipeline_state pipeline_state_object{ };
    std::vector<color_output_desc> color_outputs;
};

struct shader_program_file
{
    std::string name;
    std::vector<shader_pass_desc> passes;
    std::vector<shader_file_desc> shader_files;
};