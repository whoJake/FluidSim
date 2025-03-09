#include "main.h"

#include "compiler/glsl_compiler.h"
#include "system/device.h"

#include "pugi_include.h"
#include "shadev_channels.h"
#include "reflection.h"
#include "parser.h"
#include "system/param.h"

bool compile_program(const shader_program_file& program_file, gfx::program_def& compiled_program)
{
    compiled_program.name = program_file.name;
    compiled_program.passes.reserve(program_file.passes.size());

    for( const shader_pass_desc& pass : program_file.passes )
    {
        gfx::pass_def compiled_pass{ };
        compiled_pass.pipeline_state_object = pass.pipeline_state_object;

        compiled_pass.output_formats.color_output_count = u32_cast(pass.color_outputs.size());

        // We manually change our blend states using our pass colour outputs since they have to match.
        gfx::output_blend_states blend_states{ };
        blend_states.state_count = u32_cast(pass.color_outputs.size());

        for( u64 idx = 0; idx < pass.color_outputs.size(); idx++ )
        {
            compiled_pass.output_formats.color_outputs[idx] = pass.color_outputs[idx].format;
            blend_states.blend_states[idx] = pass.color_outputs[idx].blend_state;
        }

        compiled_pass.pipeline_state_object.set_output_blend_states(blend_states);

        if( pass.vertex_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.vertex_index = u8_cast(pass.vertex_index);
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_VERTEX;
        }
        if( pass.geometry_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.geometry_index = u8_cast(pass.geometry_index);
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_GEOMETRY;
        }
        if( pass.fragment_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.fragment_index = u8_cast(pass.fragment_index);
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_FRAGMENT;
        }
        if( pass.compute_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.compute_index = u8_cast(pass.compute_index);
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_COMPUTE;
        }

        compiled_program.passes.push_back(compiled_pass);
    }

    compiled_program.shaders.reserve(program_file.shader_files.size());
    for( const shader_file_desc& shader : program_file.shader_files )
    {
        gfx::shader_def compiled_shader{ };
        compiled_shader.entry_point = shader.entry_point;
        compiled_shader.stage = shader.stage;

        std::vector<u32> spirv_data;
        if( !compile_shader(shader.filepath.c_str(), shader.entry_point.c_str(), shader.stage, &spirv_data) )
        {
            SHADEV_ERROR("Failed to compile {}", shader.filepath);
            return false;
        }

        u64 spirv_bytes = spirv_data.size() * sizeof(u32);

        compiled_shader.data.reserve(spirv_bytes);
        compiled_shader.data.resize(spirv_bytes);
        memcpy(compiled_shader.data.data(), spirv_data.data(), spirv_bytes);

        compiled_program.shaders.push_back(std::move(compiled_shader));
    }

    return true;
}

bool compile_shader(const char* filename, const char* entry_point, gfx::shader_stage_flag_bits stage, std::vector<u32>* out_data)
{
    sys::fi_device file;
    if( !file.open(filename) )
    {
        SHADEV_ERROR("Unable to open {}", filename);
        return false;
    }

    char* file_data = new char[file.size() + 1];
    file_data[file.size()] = '\0';

    file.read((u8*)file_data, file.size());
    file.close();

    glsl_compiler compiler;
    if( !compiler.compile(stage, file_data, entry_point, out_data) )
    {
        SHADEV_ERROR("Unable to compile {}", filename);
        return false;
    }

    return true;
}

MAKEPARAM(input);
MAKEPARAM(output);

int main(int argc, const char* argv[])
{
    std::vector<const char*> args;
    for( i32 i = 1; i < argc; i++ )
    {
        args.push_back(argv[i]);
    }
    sys::param::init(args);

    if( !p_input.get() )
    {
        SHADEV_ERROR("Must supply an input path.");
        return -1;
    }

    if( !p_output.get() )
    {
        SHADEV_ERROR("Must supply an output path.");
        return -1;
    }

    shader_program_file program{ };
    if( !parser::parse_program(p_input.as_value(), &program) )
        return -1;

    gfx::program_def compiled_program{ };
    if( !compile_program(program, compiled_program) )
        return -1;

    if( !reflector::reflect(&compiled_program) )
        return -1;

#
    if( !gfx::shader_loader::save(p_output.as_value(), compiled_program) )
        return -1;

    return 0;
}