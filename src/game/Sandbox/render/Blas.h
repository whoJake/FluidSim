#pragma once
#include "TrianglePayload.h"
#include "data/bvh.h"
#include "data/ray.h"

class Blas
{
public:
    Blas(uint32_t test_spheres);

    glm::vec3 traverse(const mtl::ray& ray, const glm::vec3& sun, u32 bounces = 0) const;

    uint32_t mode{ 4 };
private:
    mtl::fixed_bvh<TrianglePayload> m_data;
};