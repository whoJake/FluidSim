#pragma once
#include "TrianglePayload.h"
#include "data/bvh.h"
#include "data/ray.h"

class Blas
{
public:
    Blas(uint32_t test_spheres);

    glm::vec3 traverse(const mtl::ray& ray) const;

    uint32_t mode{ 1 };
private:
    mtl::fixed_bvh<TrianglePayload> m_data;
};