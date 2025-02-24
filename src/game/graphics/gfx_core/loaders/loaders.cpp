#include "loaders.h"

#include "compiled_shader.h"
#include "../shader.h"

#include <fstream>
#include "system/device.h"

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

bool loaders::load(const char* filename,
                   program* out_program)
{
    GFX_ASSERT(out_program, "Must pass an allocated program.");
    sys::fi_device file;
    if( !file.open(filename) )
    {
        GFX_ERROR("Failed to open file {}.", filename);
        return false;
    }

    if( file.size() < sizeof(u32) + sizeof(compiled_program) )
    {
        GFX_ERROR("File {} is smaller than the minimum size of a program.", filename);
        return false;
    }

    u32 magic = 0;
    file.read((u8*)&magic, sizeof(u32));
    if( magic != PROGRAM_MAGIC_NUMBER )
    {
        GFX_ERROR("File {} magic number does not match that of a gfx::program", filename);
        return false;
    }

    compiled_program hdr;
    file.read((u8*)&hdr, sizeof(compiled_program));

    dt::array<compiled_shader> shdrs(hdr.shader_count);
    dt::array<compiled_pass> phdrs(hdr.pass_count);

    file.read((u8*)shdrs.data(), shdrs.size() * sizeof(compiled_shader));
    file.read((u8*)phdrs.data(), phdrs.size() * sizeof(compiled_pass));

    dt::vector<char> pname_vec(hdr.name_size);
    file.read((u8*)pname_vec.data(), hdr.name_size);

    std::string_view pname(pname_vec.data(), pname_vec.size());
    out_program->m_name = pname;
    out_program->m_passes = dt::array<pass>(hdr.pass_count);
    out_program->m_shaders = dt::array<shader>(hdr.shader_count);

    for( u64 idx = 0; idx < hdr.shader_count; idx++ )
    {
        shader& shader = out_program->m_shaders[idx];
        dt::vector<char> sname_vec(shdrs[idx].entry_point_size);
        file.read((u8*)sname_vec.data(), shdrs[idx].entry_point_size);
        std::string_view sname(sname_vec.data(), shdrs[idx].entry_point_size);

        shader.m_entryPoint = sname;
        shader.m_stage = shdrs[idx].stage;
        shader.m_code = dt::array<u32>(shdrs[idx].data_size_32);

        file.read((u8*)shader.m_code.data(), shdrs[idx].data_size_32 * sizeof(u32));
    }

    for( u64 idx = 0; idx < hdr.pass_count; idx++ )
    {
        pass& pass = out_program->m_passes[idx];
        pass.m_stageMask = phdrs[idx].stage_mask;
        pass.m_vertexShaderIndex = phdrs[idx].vertex_index;
        pass.m_geometryShaderIndex = phdrs[idx].geometry_index;
        pass.m_fragmentShaderIndex = phdrs[idx].fragment_index;
        pass.m_computeShaderIndex = phdrs[idx].compute_index;

        // Add descriptors to pass once its up and running.
        pass.m_outputs.color_output_count = 1;
        pass.m_outputs.color_outputs[0] = format::R8G8B8A8_SRGB;

        output_blend_states state{ };
        state.state_count = 1;
        pass.m_pso.set_output_blend_states(state);
    }

    file.close();
    return true;
}

} // gfx