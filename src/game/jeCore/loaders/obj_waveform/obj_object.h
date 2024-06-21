#pragma once
#include "data/fixed_vector.h"

namespace obj
{

struct vertex
{
    uint32_t position;
    uint32_t normal;
};

struct triangle
{
    vertex vertices[3];
};

class object
{
public:
    object() = default;
    object(std::string name);
    ~object();

    void add_face(const std::vector<vertex>& vertices);

    const std::vector<triangle>& get_triangles() const;
private:
    std::string m_name;
    std::vector<triangle> m_triangles;
};

} // obj