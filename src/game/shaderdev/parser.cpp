#include "parser.h"

bool parser::parse_program(const char* filename, shader_program_file* parsed_program)
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
    return parse_program(root, parsed_program);
}

bool parser::parse_program(pugi::xml_node root, shader_program_file* program)
{
    bool success = true;

    program->name = root.child("Name").child_value();

    for( pugi::xml_node item : root.child("Passes").children() )
    {
        shader_pass_desc pass{ };
        if( parse_pass(item, &pass) )
        {
            program->passes.push_back(pass);
            continue;
        }

        success = false;
    }

    for( pugi::xml_node item : root.child("Shaders").children() )
    {
        shader_file_desc file{ };
        if( parse_shader(item, &file) )
        {
            program->shader_files.push_back(file);
            continue;
        }

        success = false;
    }

    return success;
}

bool parser::parse_pass(pugi::xml_node root, shader_pass_desc* pass)
{
    if( pugi::xml_node vertex = root.child("VertexShader") )
    {
        pass->vertex_index = std::stoi(vertex.child_value());
    }
    if( pugi::xml_node geometry = root.child("GeometryShader") )
    {
        pass->geometry_index = std::stoi(geometry.child_value());
    }
    if( pugi::xml_node fragment = root.child("FragmentShader") )
    {
        pass->fragment_index = std::stoi(fragment.child_value());
    }
    if( pugi::xml_node compute = root.child("ComputeShader") )
    {
        pass->compute_index = std::stoi(compute.child_value());
    }

    for( pugi::xml_node item : root.child("ColorOutputs").children() )
    {
        color_output_desc color_output{ };
        pugi::xml_node format = item.child("Format");
        SHADEV_ASSERT(format, "Color output must have a format.");

        color_output.format = parse_format(format.child_value());
        pass->color_outputs.push_back(color_output);

        // TODO: color_blend_state
    }

    bool success = true;
    if( root.child("Settings") )
    {
        success = parse_pass_pso(root.child("Settings"), &pass->pipeline_state_object);
    }

    return success;
}

bool parser::parse_pass_pso(pugi::xml_node root, gfx::pipeline_state* pso)
{
    gfx::rasterization_state rast_state = pso->get_rasterization_state();

    {
        if( pugi::xml_node node = root.child("PolygonMode") )
        {
            if( !strcmp(node.child_value(), "Fill") )
                rast_state.polygon_mode = gfx::polygon_mode::fill;
            else if( !strcmp(node.child_value(), "Line") )
                rast_state.polygon_mode = gfx::polygon_mode::line;
            else if( !strcmp(node.child_value(), "Point") )
                rast_state.polygon_mode = gfx::polygon_mode::point;
        }
        if( pugi::xml_node node = root.child("LineWidth") )
        {
            rast_state.line_width = std::stof(node.child_value());
        }
    }

    pso->set_rasterization_state(rast_state);
    return true;
}

bool parser::parse_shader(pugi::xml_node root, shader_file_desc* shader)
{
    shader->filepath = root.child("File").child_value();
    shader->entry_point = root.child("EntryPoint").child_value();

    std::string stage = root.child("Stage").child_value();
    if( stage == "VERTEX" )
    {
        shader->stage = gfx::SHADER_STAGE_VERTEX;
    }
    else if( stage == "GEOMETRY" )
    {
        shader->stage = gfx::SHADER_STAGE_GEOMETRY;
    }
    else if( stage == "FRAGMENT" )
    {
        shader->stage = gfx::SHADER_STAGE_FRAGMENT;
    }
    else if( stage == "COMPUTE" )
    {
        shader->stage = gfx::SHADER_STAGE_COMPUTE;
    }

    return true;
}