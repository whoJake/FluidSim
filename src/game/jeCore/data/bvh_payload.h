#pragma once

#include "ray.h"
#include "aabb.h"

namespace mtl
{
class bvh_payload
{
public:
    virtual ~bvh_payload() = default;
    
    virtual float get_cost() const = 0;
    virtual glm::vec3 get_point() const = 0;
    virtual mtl::aabb3 get_bounds() const = 0;

    virtual bool check_ray(const ray& ray, ray_hit_info* hit) const = 0;
protected:
    bvh_payload() = default;
    bvh_payload(bvh_payload&&) = default;
    bvh_payload(const bvh_payload&) = default;
    bvh_payload& operator=(bvh_payload&&) = default;
    bvh_payload& operator=(const bvh_payload&) = default;
};
} // mtl