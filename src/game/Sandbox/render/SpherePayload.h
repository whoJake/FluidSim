#pragma once

#include "data/bvh.h"

class SpherePayload : public mtl::bvh_payload
{
public:
    SpherePayload(glm::vec3 position);
    ~SpherePayload() = default;

    SpherePayload() = default;
    SpherePayload(SpherePayload&&) = default;
    SpherePayload(const SpherePayload&) = default;
    SpherePayload& operator=(SpherePayload&&) = default;
    SpherePayload& operator=(const SpherePayload&) = default;

    float get_cost() const override;
    glm::vec3 get_point() const override;
    mtl::aabb3 get_bounds() const override;

    bool check_ray(const mtl::ray& ray, mtl::ray_hit_info* hit) const override;
private:
    mtl::aabb3 calculate_bounds() const;
private:
    glm::vec3 m_position;
    float m_radius;
    glm::vec3 m_color;

    mtl::aabb3 m_bounds;
};