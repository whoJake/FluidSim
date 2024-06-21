#pragma once

#include "data/bvh.h"

class TrianglePayload : public mtl::bvh_payload
{
public:
    TrianglePayload(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 n1, glm::vec3 n2, glm::vec3 n3);
    ~TrianglePayload() = default;

    TrianglePayload() = default;
    TrianglePayload(TrianglePayload&&) = default;
    TrianglePayload(const TrianglePayload&) = default;
    TrianglePayload& operator=(TrianglePayload&&) = default;
    TrianglePayload& operator=(const TrianglePayload&) = default;

    float get_cost() const override;
    glm::vec3 get_point() const override;
    mtl::aabb3 get_bounds() const override;

    bool check_ray(const mtl::ray& ray, mtl::ray_hit_info* hit) const override;
private:
    mtl::aabb3 calculate_bounds() const;
private:
    glm::vec3 m_vertices[3];
    glm::vec3 m_normals[3];

    glm::vec3 m_color;
    mtl::aabb3 m_bounds;
};