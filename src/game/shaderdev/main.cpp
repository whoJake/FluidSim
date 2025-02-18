#include "main.h"

#include "compiler/glsl_compiler.h"
#include "system/device.h"

#include "pugi_include.h"

#include <fstream>

bool parse_program(const char* filename, shader_program_file& program)
{
    pugi::xml_document xDoc;

    pugi::xml_parse_result result = xDoc.load_file(filename);
    if( !result )
    {
        SHADEV_ERROR("Parsing error in program {}.", filename);
        SHADEV_ERROR("Error at offset {}: {}", result.offset, result.description());
        return false;
    }

    pugi::xml_node root = xDoc.child("Program");
    
    program.name = root.child("Name").child_value();

    // Parse our passes
    for( pugi::xml_node item : root.child("Passes").children() )
    {
        shader_pass_desc pass{ };
        if( pugi::xml_node vertex = item.child("VertexShader") )
        {
            pass.vertex_index = std::stoi(vertex.child_value());
        }
        if( pugi::xml_node geometry = item.child("GeometryShader") )
        {
            pass.geometry_index = std::stoi(geometry.child_value());
        }
        if( pugi::xml_node fragment = item.child("GeometryShader") )
        {
            pass.fragment_index = std::stoi(fragment.child_value());
        }
        if( pugi::xml_node compute = item.child("GeometryShader") )
        {
            pass.compute_index = std::stoi(compute.child_value());
        }
        
        program.passes.push_back(pass);
    }

    // Parse our shader files
    for( pugi::xml_node item : root.child("Shaders").children() )
    {
        shader_file_desc file{ };
        file.filepath = item.child("File").child_value();
        file.entry_point = item.child("EntryPoint").child_value();

        std::string stage = item.child("Stage").child_value();
        if( stage == "VERTEX" )
        {
            file.stage = gfx::SHADER_STAGE_VERTEX;
        }
        else if( stage == "GEOMETRY" )
        {
            file.stage = gfx::SHADER_STAGE_GEOMETRY;
        }
        else if( stage == "FRAGMENT" )
        {
            file.stage = gfx::SHADER_STAGE_FRAGMENT;
        }
        else if( stage == "COMPUTE" )
        {
            file.stage = gfx::SHADER_STAGE_COMPUTE;
        }

        program.shader_files.push_back(std::move(file));
    }

    return true;
}

bool compile_program(const shader_program_file& program_file, shader_program_compiled& compiled_program)
{
    compiled_program.name = program_file.name;
    compiled_program.passes.reserve(program_file.passes.size());

    for( const shader_pass_desc& pass : program_file.passes )
    {
        shader_pass_compiled compiled_pass{ };
        if( pass.vertex_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.vertex_index = pass.vertex_index;
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_VERTEX;
        }
        if( pass.geometry_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.geometry_index = pass.geometry_index;
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_GEOMETRY;
        }
        if( pass.fragment_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.fragment_index = pass.fragment_index;
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_FRAGMENT;
        }
        if( pass.compute_index != shader_pass_desc::invalid_index )
        {
            compiled_pass.compute_index = pass.compute_index;
            compiled_pass.stage_mask |= gfx::SHADER_STAGE_COMPUTE;
        }

        compiled_program.passes.push_back(compiled_pass);
    }

    compiled_program.shaders.reserve(program_file.shader_files.size());
    for( const shader_file_desc& shader : program_file.shader_files )
    {
        shader_compiled compiled_shader{ };
        compiled_shader.entry_point = shader.entry_point;
        compiled_shader.stage = shader.stage;

        if( !compile_shader(shader.filepath.c_str(), shader.entry_point.c_str(), shader.stage, &compiled_shader.data) )
        {
            SHADEV_ERROR("Failed to compile {}", shader.filepath);
            return false;
        }

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

bool write_program(const char* filename, const shader_program_compiled& compiled_program)
{
    std::fstream file;
    file.open(filename, std::ios::binary | std::ios::trunc | std::ios::out);

    compiled_header hdr{ };
    hdr.m_nameSize = u8_cast(compiled_program.name.size());
    hdr.m_shaderCount = u8_cast(compiled_program.shaders.size());
    hdr.m_passCount = u8_cast(compiled_program.passes.size());
    hdr.m_pad = 0;

    file.write((const char*)&hdr, sizeof(compiled_header));

    u32 offset = hdr.m_nameSize;
    for( const shader_compiled& shader : compiled_program.shaders )
    {
        compiled_shader_header shdr{ };
        shdr.m_entryPointOffset = offset;
        shdr.m_entryPointSize = u8_cast(shader.entry_point.size());
        shdr.m_stage = shader.stage;
        shdr.m_dataSize32 = u32_cast(shader.data.size());

        offset += shdr.m_entryPointSize + (shdr.m_dataSize32 * sizeof(u32));

        file.write((const char*)&shdr, sizeof(compiled_shader_header));
    }

    for( const shader_pass_compiled& pass : compiled_program.passes )
    {
        // Exactly the same for now.
        compiled_pass_header phdr{ };
        phdr.stage_mask = pass.stage_mask;
        phdr.vertex_index = pass.vertex_index;
        phdr.geometry_index = pass.geometry_index;
        phdr.fragment_index = pass.fragment_index;
        phdr.compute_index = pass.compute_index;

        file.write((const char*)&phdr, sizeof(compiled_pass_header));
    }

    // Write our "heap" data
    {
        file.write(compiled_program.name.data(), hdr.m_nameSize);
        
        for( const shader_compiled& shader : compiled_program.shaders )
        {
            file.write(shader.entry_point.data(), shader.entry_point.size());
            file.write((const char*)shader.data.data(), shader.data.size() * sizeof(u32));
        }
    }

    file.close();
    return true;
}

int main(int argc, const char* argv[])
{
    shader_program_file program{ };
    if( !parse_program("source/programs/triangle.fxp", program) )
    {
        return -1;
    }

    shader_program_compiled compiled_program{ };
    if( !compile_program(program, compiled_program) )
    {
        return -1;
    }

    if( !write_program("compiled/triangle.fxcp", compiled_program) )
    {
        return -1;
    }

    return 0;
}