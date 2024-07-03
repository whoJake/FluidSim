#include "Blas.h"

#include <cstdlib>
#include "device/fiDevice.h"
#include "loaders/obj_waveform.h"
#include "system/timer.h"

Blas::Blas(uint32_t test_spheres) :
    m_data(test_spheres),
    m_data2(test_spheres)
{
    srand((unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count());

    mtl::aabb3 random_range
    {
        { -10.f, -10.f, -30.f },
        { 10.f, 10.f, 10.f }
    };

    fiDevice device;
    // device.open("assets/obj/12140_Skull_v3_L2.obj");
    device.open("assets/obj/apple.obj");

    obj::file obj_file;
    obj_file.parse(device);

    const mtl::fixed_vector<glm::vec3>& vertices = obj_file.get_vertices();
    const mtl::fixed_vector<glm::vec3>& normals = obj_file.get_normals();
    const mtl::fixed_vector<obj::object>& objects = obj_file.get_objects();

    mtl::fixed_vector<SpherePayload> spheres(1);
    spheres[0] = SpherePayload(glm::vec3(0.f, 0.f, 0.f));
    m_data = mtl::fixed_bvh<SpherePayload>(std::move(spheres));

    std::vector<obj::triangle> triangleObjs;
    for( const obj::object& obj : objects )
    {
        for( const obj::triangle& triangle : obj.get_triangles() )
        {
            triangleObjs.push_back(triangle);
        }
    }

    mtl::fixed_vector<TrianglePayload> triangles(triangleObjs.size());
    for( size_t i = 0; i < triangles.size(); i++ )
    {
        obj::vertex v1 = triangleObjs[i].vertices[0];
        obj::vertex v2 = triangleObjs[i].vertices[1];
        obj::vertex v3 = triangleObjs[i].vertices[2];

        glm::vec3 mult{ 1.f, 1.f, 1.f };

        triangles[i] = TrianglePayload(
            vertices[v1.position - 1] * mult,
            vertices[v2.position - 1] * mult,
            vertices[v3.position - 1] * mult,
            normals[v1.normal - 1] * mult,
            normals[v2.normal - 1] * mult,
            normals[v3.normal - 1] * mult
        );
    }

    m_data2 = mtl::fixed_bvh<TrianglePayload>(std::move(triangles));

    mtl::bvh_build_settings settings{ };
    settings.maxDepth = 10;
    settings.minPayloadsPerNode = 1;
    m_data.build(&settings);
    m_data2.build(&settings);
}

glm::vec3 Blas::traverse(const mtl::ray& ray) const
{
    bool sucess = false;

    mtl::bvh_traverse_options options{ };
    options.max_bounces = 1;

    mtl::bvh_traverse_stats stats{ };
    mtl::bvh_traverse_output output = m_data2.traverse(ray, options, &stats);

    return output.diffuse;
}