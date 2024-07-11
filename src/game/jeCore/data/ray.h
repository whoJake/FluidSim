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

    inline glm::vec2 intersects(const mtl::aabb3& bound) const
    {
        float tx1 = (bound.min.x - position.x) / direction.x;
        float tx2 = (bound.max.x - position.x) / direction.x;
        float tmin = std::min(tx1, tx2);
        float tmax = std::max(tx1, tx2);
        float ty1 = (bound.min.y - position.y) / direction.y;
        float ty2 = (bound.max.y - position.y) / direction.y;
        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));
        float tz1 = (bound.min.z - position.z) / direction.z;
        float tz2 = (bound.max.z - position.z) / direction.z;
        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));
        return { tmin, tmax };
    }
};

} // mtl