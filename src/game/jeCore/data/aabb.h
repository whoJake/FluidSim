#pragma once

#include <cstdlib>

namespace mtl
{

class aabb3
{
public:
    constexpr glm::vec3 centre() const
    {
        return (min + max) / 2.f;
    }

    constexpr glm::vec3 size() const
    {
        return (max - min);
    }
    
    constexpr glm::vec3 extent() const
    {
        return size() / 2.f;
    }

    constexpr bool contains(const glm::vec3& point) const
    {
        return point.x >= min.x && point.x <= max.x
            && point.y >= min.y && point.y <= max.y
            && point.z >= min.z && point.z <= max.z;
    }

    constexpr bool contains(const glm::vec3& point, float radius) const
    {
        float dsqr = radius * radius;

        /* assume C1 and C2 are element-wise sorted, if not, do that now */
        if (point.x < min.x) dsqr -= (point.x - min.x) * (point.x - min.x);
        else if (point.x > max.x) dsqr -= (point.x - max.x) * (point.x - max.x);
        if (point.y < min.y) dsqr -= (point.y - min.y) * (point.y - min.y);
        else if (point.y > max.y) dsqr -= (point.y - max.y) * (point.y - max.y);
        if (point.z < min.z) dsqr -= (point.z - min.z) * (point.z - min.z);
        else if (point.z > max.z) dsqr -= (point.z - max.z) * (point.z - max.z);
        return dsqr > 0;
    }

    constexpr bool contains(const aabb3& other) const
    {
        return other.min.x >= min.x && other.max.x <= max.x
            && other.min.y >= min.y && other.max.y <= max.y
            && other.min.z >= min.z && other.max.z <= max.z;
    }

    constexpr bool intersects(const aabb3& other) const
    {
        return other.min.x <= max.x && other.max.x >= min.x
            && other.min.y <= max.y && other.max.y >= min.y
            && other.min.z <= max.z && other.max.z >= min.z;
    }

    constexpr auto operator<=>(const aabb3& other) const = default;

    constexpr void expand_to_fit(const glm::vec3& point)
    {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);

        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    constexpr void expand_to_fit(const aabb3& other)
    {
        expand_to_fit(other.min);
        expand_to_fit(other.max);
    }

    inline glm::vec3 random_point_inside() const
    {
        return
        {
            min.x + (max.x - min.x) * (rand() / static_cast<float>(RAND_MAX)),
            min.y + (max.y - min.y) * (rand() / static_cast<float>(RAND_MAX)),
            min.z + (max.z - min.z) * (rand() / static_cast<float>(RAND_MAX))
        };
    }

    inline float get_surface_area() const
    {
        glm::vec3 size = max - min;
        float x = 2 * (size.y * size.z);
        float y = 2 * (size.x * size.z);
        float z = 2 * (size.x * size.y);
        return x + y + z;
    }

    inline float get_sah_cost() const
    {
        // half of get_surface area, since theoretically you can only see half of the box's surface area at once

        glm::vec3 size = max - min;
        float x = size.y * size.z;
        float y = size.x * size.z;
        float z = size.x * size.y;
        return x + y + z;
    }
public:
    glm::vec3 min;
    glm::vec3 max;
};

inline static aabb3 aabb3_empty
{
    { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() },
    { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() }
};

} // mtl