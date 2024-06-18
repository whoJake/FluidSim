#include "Blas.h"

#include <cstdlib>

Blas::Blas() :
    m_min(-20.f, -20.f, -20.f),
    m_max(20.f, 20.f, 20.f)
{
    size_t count = 10;

    srand((unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    for( size_t i = 0; i < count; i++ )
    {
        add_random_sphere();
    }
}

bool Blas::traverse(const Ray& ray, HitInfo* hit) const
{
    bool sucess = false;

    *hit = HitInfo{ };
    hit->distance = std::numeric_limits<float>::max();

    for( const Sphere& sphere : m_data )
    {
        float distance = ray.intersects(sphere.position, sphere.radius);
        if( distance > 0.f
            && distance < hit->distance )
        {
            hit->distance = distance;
            hit->normal = glm::normalize((ray.position + (ray.direction * distance)) - sphere.position);
            hit->color = sphere.color;

            sucess = true;
        }
    }

    return sucess;
}

void Blas::add_random_sphere()
{
    glm::vec3 pos
    {
        m_min.x + (m_max.x - m_min.x) * (rand() / static_cast<float>(RAND_MAX)),
        m_min.y + (m_max.y - m_min.y) * (rand() / static_cast<float>(RAND_MAX)),
        m_min.z + (m_max.z - m_min.z) * (rand() / static_cast<float>(RAND_MAX))
    };

    glm::vec4 color
    {
        glm::clamp((rand() / static_cast<float>(RAND_MAX)) * 2.f, 0.f, 1.f),
        glm::clamp((rand() / static_cast<float>(RAND_MAX)) * 2.f, 0.f, 1.f),
        glm::clamp((rand() / static_cast<float>(RAND_MAX)) * 2.f, 0.f, 1.f),
        1.f
    };

    float radius = 0.5f + 6.f * (rand() / static_cast<float>(RAND_MAX));

    m_data.emplace_back(pos, radius, color);
}