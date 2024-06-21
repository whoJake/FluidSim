#pragma once
#include "SpherePayload.h"
#include "TrianglePayload.h"
#include "data/bvh.h"
#include "data/ray.h"

class Blas
{
public:
    Blas(uint32_t test_spheres);

    glm::vec3 traverse(const mtl::ray& ray) const;
private:
    mtl::fixed_bvh<SpherePayload> m_data;
    mtl::fixed_bvh<TrianglePayload> m_data2;
};