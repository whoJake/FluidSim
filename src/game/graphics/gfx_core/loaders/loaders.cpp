#include "loaders.h"

#include "compiled_shader.h"
#include "../shader.h"

#include <fstream>

namespace gfx
{

bool loaders::save(const char* filename,
                   const char* program_name,
                   const compiled_program& program,
                   const std::vector<compiled_pass>& pass_descriptions,
                   const std::vector<compiled_shader>& shader_descriptions,
                   const std::vector<const char*>& shader_entry_points,
                   const std::vector<std::vector<u32>>& shader_datas)
{
    GFX_ASSERT(strlen(program_name) == program.name_size, "Supplied program name is not the same size as program.name_size.");
    GFX_ASSERT(program.pass_count == pass_descriptions.size(), "Program pass count does not match the number of pass_descriptions.");
    GFX_ASSERT(program.shader_count == shader_descriptions.size(), "Program shader count does not match the number of shader_descriptions.");
    GFX_ASSERT(program.shader_count == shader_entry_points.size(), "Program shader count does not match the number of shader_entry_points.");
    GFX_ASSERT(program.shader_count == shader_datas.size(), "Program shader count does not match the number of shader_datas.");

    // Validate
    for( u32 shaderIdx = 0; shaderIdx < program.shader_count; shaderIdx++ )
    {
        GFX_ASSERT(shader_descriptions[shaderIdx].entry_point_size == strlen(shader_entry_points[shaderIdx]),
            "Program shader at index {} has entry_point_size that does not match shader_entry_points[{}]", shaderIdx, shaderIdx);
        GFX_ASSERT(shader_descriptions[shaderIdx].data_size_32 == shader_datas[shaderIdx].size(),
            "Program shader at index {} has data_size_32 that does not match shader_datas[{}]", shaderIdx, shaderIdx);
    }

    std::fstream out_file;
    out_file.open(filename, std::ios::binary | std::ios::trunc | std::ios::out);
    if( !out_file.is_open() )
    {
        GFX_ERROR("Failed to save compiled shader {} at filename {}.", program_name, filename);
        return false;
    }

    const u32 magic = PROGRAM_MAGIC_NUMBER;
    out_file.write((const char*)&magic, sizeof(u32));

    out_file.write((const char*)&program, sizeof(compiled_program));

    u32 shader_offset = program.name_size;
    // We take a copy here since we have to modify entry_point_offset
    for( compiled_shader comp_shader : shader_descriptions )
    {
        comp_shader.entry_point_offset = shader_offset;

        out_file.write((const char*)&comp_shader, sizeof(compiled_shader));
        shader_offset += comp_shader.entry_point_size + (comp_shader.data_size_32 * sizeof(u32));
    }

    for( const compiled_pass& comp_pass : pass_descriptions )
    {
        out_file.write((const char*)&comp_pass, sizeof(compiled_pass));
    }

    // Write our heap data
    out_file.write(program_name, program.name_size);
    
    for( u32 shaderIdx = 0; shaderIdx < program.shader_count; shaderIdx++ )
    {
        const compiled_shader& description = shader_descriptions[shaderIdx];
        out_file.write(shader_entry_points[shaderIdx], description.entry_point_size);
        out_file.write((const char*)shader_datas[shaderIdx].data(), description.data_size_32 * sizeof(u32));
    }

    bool success = out_file.good();
    out_file.close();

    return success;

}

} // gfx