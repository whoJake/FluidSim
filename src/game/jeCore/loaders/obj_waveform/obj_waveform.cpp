#include "loaders/obj_waveform.h"

#include "helpers/string_manip.h"

namespace obj
{

file::file() :
    m_objects()
{ }

file::~file()
{ }

bool file::parse(fiDevice& device)
{
    if( !device.is_open() )
    {
        return false;
    }

    calculate_buffer_sizes(device);

    device.seek_to_start();
    std::vector<uint8_t> buffer;

    size_t vertexIndex{ 0 };
    size_t normalIndex{ 0 };
    size_t objectContext{ 0 };

    while( device.read_line(&buffer) )
    {
        if( buffer.size() < 2 )
        {
            // blank line
            continue;
        }

        std::string line(buffer.begin(), buffer.end());

        // comment
        if( line.starts_with('#') )
        {
            continue;
        }

        if( line.starts_with('g') )
        {
            objectContext++;
            continue;
        }

        if( line.starts_with("v ") )
        {
            parse_vertex(vertexIndex, line);
            vertexIndex++;
            continue;
        }

        if( line.starts_with("vn") )
        {
            parse_normal(normalIndex, line);
            normalIndex++;
            continue;
        }

        if( line.starts_with("f ") )
        {
            parse_face(objectContext - 1, line);
            continue;
        }
    }

    return true;
}

void file::calculate_buffer_sizes(fiDevice& device)
{
    size_t vertexCount{ 0 };
    size_t normalCount{ 0 };
    size_t texCoordCount{ 0 };
    std::vector<std::string> objectNames;

    std::vector<uint8_t> buffer;
    while( device.read_line(&buffer) )
    {
        std::string line(buffer.begin(), buffer.end());

        if( line.starts_with('#') )
        {
            continue;
        }
        else if( line.starts_with("v ") )
        {
            vertexCount++;
        }
        else if( line.starts_with("vn") )
        {
            normalCount++;
        }
        else if( line.starts_with("vt") )
        {
            texCoordCount++;
        }
        else if( line.starts_with("g") )
        {
            objectNames.push_back(line.substr(2) );
        }
    }

    m_vertices = mtl::fixed_vector<glm::vec3>(vertexCount);
    m_normals = mtl::fixed_vector<glm::vec3>(normalCount);
    m_objects = mtl::fixed_vector<object>(objectNames.size());

    for( size_t i = 0; i < objectNames.size(); i++ )
    {
        m_objects[i] = object(objectNames[i]);
    }
}

const mtl::fixed_vector<glm::vec3>& file::get_vertices() const
{
    return m_vertices;
}

const mtl::fixed_vector<glm::vec3>& file::get_normals() const
{
    return m_normals;
}

const mtl::fixed_vector<object>& file::get_objects() const
{
    return m_objects;
}

void file::parse_vertex(size_t context, const std::string& line)
{
    std::vector<std::string> split = split_string(line, " ");
    size_t vecPart{ 0 };

    for( std::string& part : split )
    {
        if( vecPart >= 3 )
        {
            return; // more than 3 positions?
        }

        if( !part.starts_with('v') )
        {
            float value = std::stof(part);

            m_vertices[context][vecPart] = value;
            vecPart++;
        }
    }
}

void file::parse_normal(size_t context, const std::string& line)
{
    std::vector<std::string> split = split_string(line, " ");
    size_t vecPart{ 0 };

    for( std::string& part : split )
    {
        if( vecPart >= 3 )
        {
            return; // more than 3 normals?
        }

        if( !part.starts_with("vn") )
        {
            float value = std::stof(part);

            m_normals[context][vecPart] = value;
            vecPart++;
        }
    }
}

void file::parse_face(size_t context, const std::string& line)
{
    std::vector<std::string> split = split_string(line, " ");
    std::vector<vertex> defines;

    for( std::string& part : split )
    {
        if( !part.starts_with('f') )
        {
            std::vector<std::string> components = split_string(part, "/");

            if( components.size() < 3 )
            {
                continue;
            }

            vertex v
            {
                std::stoi(components[0]),
                std::stoi(components[2])
            };

            defines.push_back(v);
        }
    }

    m_objects[context].add_face(defines);
}

} // obj