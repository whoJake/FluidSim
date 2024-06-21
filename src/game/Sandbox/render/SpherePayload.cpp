#include "SpherePayload.h"

SpherePayload::SpherePayload(glm::vec3 position) :
    m_position(position),
    m_radius(),
    m_color(),
    m_bounds()
{
    m_color =
    {
        glm::clamp((rand() / static_cast<float>(RAND_MAX)), 0.f, 1.f),
        glm::clamp((rand() / static_cast<float>(RAND_MAX)), 0.f, 1.f),
        glm::clamp((rand() / static_cast<float>(RAND_MAX)), 0.f, 1.f)
    };

    m_radius = 0.3f;
    m_bounds = calculate_bounds();
}

float SpherePayload::get_cost() const
{
    return m_bounds.get_surface_area();
}

glm::vec3 SpherePayload::get_point() const
{
    return m_position;
}

mtl::aabb3 SpherePayload::get_bounds() const
{
    return m_bounds;
}

mtl::aabb3 SpherePayload::calculate_bounds() const
{
    glm::vec3 extent{ m_radius, m_radius, m_radius };

    return
    {
        m_position - extent,
        m_position + extent
    };
}

bool SpherePayload::check_ray(const mtl::ray& ray, mtl::ray_hit_info* hit) const
{
    float distance = ray.intersects(m_position, m_radius);
    if( distance < 0.f )
        return false;

    hit->distance = distance;
    hit->diffuse = m_color;
    hit->normal = glm::normalize((ray.position + (ray.direction * distance)) - m_position);
    return true;
}