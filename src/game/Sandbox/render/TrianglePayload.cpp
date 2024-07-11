#include "TrianglePayload.h"

TrianglePayload::TrianglePayload(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n1, glm::vec3 n2, glm::vec3 n3) :
    m_vertices{ v1, v2, v3 },
    m_normals{ n1, n2, n3 },
    m_color(),
    m_bounds()
{
    m_color =
    {
        glm::clamp((rand() / static_cast<float>(RAND_MAX)), 0.f, 1.f),
        glm::clamp((rand() / static_cast<float>(RAND_MAX)), 0.f, 1.f),
        glm::clamp((rand() / static_cast<float>(RAND_MAX)), 0.f, 1.f)
    };

    // m_color = glm::vec3(1.f, 0.f, 0.f);

    glm::vec3 cross = glm::cross(m_vertices[1] - m_vertices[2], m_vertices[0] - m_vertices[2]);
    glm::vec3 normal = glm::normalize(cross);
    m_normals[0] = normal;
    m_normals[1] = normal;
    m_normals[2] = normal;

    m_bounds = calculate_bounds();
}

float TrianglePayload::get_cost() const
{
    return m_bounds.get_surface_area();
}

glm::vec3 TrianglePayload::get_point() const
{
    return (m_vertices[0] + m_vertices[1] + m_vertices[2]) / 3.f;
}

mtl::aabb3 TrianglePayload::get_bounds() const
{
    return m_bounds;
}

mtl::aabb3 TrianglePayload::calculate_bounds() const
{
    mtl::aabb3 retval
    {
        m_vertices[0],
        m_vertices[1]
    };
    retval.expand_to_fit(m_vertices[2]);

    return retval;
}

bool TrianglePayload::check_ray(const mtl::ray& ray, mtl::ray_hit_info* hit) const
{
    glm::vec3 edge1 = m_vertices[1] - m_vertices[0];
    glm::vec3 edge2 = m_vertices[2] - m_vertices[0];
    glm::vec3 h = glm::cross( ray.direction, edge2 );

    float a = glm::dot(edge1, h);

    if ( a > -0.0001f && a < 0.0001f ) return false;

    float f = 1.f / a;
    glm::vec3 s = ray.position - m_vertices[0];
    float u = f * glm::dot(s, h);

    if (u < 0.f || u > 1.f ) return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.direction, q);

    if (v < 0.f || u + v > 1.f) return false;

    float t = f * glm::dot(edge2, q);
    hit->distance = t;
    hit->diffuse = m_color;
    hit->normal = sample_normals(ray.position + (ray.direction * t));
    return true;
}

glm::vec3 TrianglePayload::sample_normals(glm::vec3 position) const
{
    glm::vec3 result;

    glm::vec3 a2b = m_vertices[1] - m_vertices[0];
    glm::vec3 a2x = position - m_vertices[0];
    float a2bx = glm::dot(a2b, a2x);
    result = m_normals[0] + (m_normals[1] - m_normals[0]) * a2bx;

    glm::vec3 xa2b = m_vertices[0] + (a2b * a2bx);
    float px2c = glm::dot(xa2b, m_vertices[2]);

    return result + (m_normals[2] - result) * px2c;
}

glm::vec3 TrianglePayload::get_color() const
{
    return m_color;
}