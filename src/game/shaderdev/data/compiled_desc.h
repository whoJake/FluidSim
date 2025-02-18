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

/// -----------------------------------------------------------------------------------------------------------------------------
/// | MAGIC | compiled_header | compiled_shader_desc N | shader_pass_compiled N | program_name | | entry_point | spirv_data | N |
/// -----------------------------------------------------------------------------------------------------------------------------
struct compiled_header
{
    static constexpr u32 MAGIC_NUMBER = MAKE_MAGIC_NUMBER('F', 'X', 'C', 'P');

    u32 m_magic = MAGIC_NUMBER;
    // Name implictly starts at 0
    u8 m_nameSize;

    u8 m_shaderCount;
    u8 m_passCount;
    u8 m_pad;
};

struct compiled_shader_header
{
    u32 m_entryPointOffset;
    u8 m_entryPointSize;
    u8 m_pad[3];
    u32 m_stage;
    u32 m_dataSize32;
};

struct compiled_pass_header
{
    gfx::shader_stage_flags stage_mask;

    u8 vertex_index;
    u8 geometry_index;
    u8 fragment_index;
    u8 compute_index;
};