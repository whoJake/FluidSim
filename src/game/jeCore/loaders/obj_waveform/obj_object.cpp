#include "obj_object.h"

namespace obj
{

object::object(std::string name) :
    m_name(name),
    m_triangles()
{ }

object::~object()
{ }

void object::add_face(const std::vector<vertex>& vertices)
{
    if( vertices.size() < 3 )
    {
        return;
    }

    if( vertices.size() == 3 )
    {
        triangle f
        {
            vertices[0],
            vertices[1],
            vertices[2]
        };
        m_triangles.push_back(f);
        return;
    }

    if( vertices.size() == 4 )
    {
        triangle f1
        {
            vertices[0],
            vertices[1],
            vertices[2]
        };

        triangle f2
        {
            vertices[2],
            vertices[3],
            vertices[0]
        };

        m_triangles.push_back(f1);
        m_triangles.push_back(f2);
        return;
    }

    return;
}

const std::vector<triangle>& object::get_triangles() const
{
    return m_triangles;
}

} // obj