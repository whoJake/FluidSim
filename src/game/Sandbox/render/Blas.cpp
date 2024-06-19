#include "Blas.h"

#include <cstdlib>

Blas::Blas(uint32_t test_spheres) :
    m_data(test_spheres)
{
    srand((unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count());

    mtl::aabb3 random_range
    {
        { -10.f, -10.f, -30.f },
        { 10.f, 10.f, 10.f }
    };

    mtl::fixed_vector<SpherePayload> spheres(test_spheres, random_range);
    m_data = mtl::fixed_bvh<SpherePayload>(std::move(spheres));

    mtl::bvh_build_settings settings{ };
    settings.maxDepth = 10;
    settings.minPayloadsPerNode = 1;
    m_data.build(&settings);
}

glm::vec3 Blas::traverse(const mtl::ray& ray) const
{
    bool sucess = false;

    mtl::bvh_traverse_options options{ };
    options.max_bounces = 5;

    mtl::bvh_traverse_stats stats{ };
    mtl::bvh_traverse_output output = m_data.traverse(ray, options, &stats);

    return output.diffuse;
}