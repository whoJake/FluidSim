#pragma once

#include "aabb.h"

namespace mtl
{

struct ray_hit_info
{
    float distance;
    glm::vec3 diffuse;
    glm::vec3 normal;
};

struct ray
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 invDirection;

    inline float intersects(glm::vec3 sphere_pos, float sphere_radius) const
    {
        glm::vec3 oc = position - sphere_pos;
        float a = glm::dot(direction, direction);
        float b = 2.f * dot(oc, direction);
        float c = dot(oc, oc) - sphere_radius * sphere_radius;
        float disc = b * b - 4 * a * c;

        if( disc < 0 )
        {
            return -1.f;
        }

        return (-b - sqrt(disc)) / (2.f * a);
    }

    inline bool intersects(const mtl::aabb3& bounds, f32* distance = nullptr) const
    {
        f32 t1 = (bounds.min.x - position.x) * invDirection.x;
        f32 t2 = (bounds.max.x - position.x) * invDirection.x;
        f32 t3 = (bounds.min.y - position.y) * invDirection.y;
        f32 t4 = (bounds.max.y - position.y) * invDirection.y;
        f32 t5 = (bounds.min.z - position.z) * invDirection.z;
        f32 t6 = (bounds.max.z - position.z) * invDirection.z;

        f32 tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
        f32 tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

        if( tmax < 0.f )
            return false;

        if( tmin > tmax )
            return false;

        if( distance )
            *distance = tmin;
        return true;
    }
};

} // mtl