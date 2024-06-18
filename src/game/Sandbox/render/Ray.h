#pragma once

struct Ray
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
};