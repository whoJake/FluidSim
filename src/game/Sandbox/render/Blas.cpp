#include "Blas.h"

#include <cstdlib>
#include "device/fiDevice.h"
#include "loaders/obj_waveform.h"
#include "system/timer.h"

Blas::Blas(uint32_t test_spheres) :
    m_data(test_spheres)
{
    srand((unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count());

    mtl::aabb3 random_range
    {
        { -10.f, -10.f, -30.f },
        { 10.f, 10.f, 10.f }
    };

    fiDevice device;
    device.open("assets/obj/12140_Skull_v3_L2.obj");
    // device.open("assets/obj/apple.obj");

    obj::file obj_file;
    obj_file.parse(device);

    const mtl::fixed_vector<glm::vec3>& vertices = obj_file.get_vertices();
    const mtl::fixed_vector<glm::vec3>& normals = obj_file.get_normals();
    const mtl::fixed_vector<obj::object>& objects = obj_file.get_objects();

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

        glm::vec3 rotation{ -90.f, 0.f, 0.f };
        glm::quat rot(glm::vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)));

        triangles[i] = TrianglePayload(
            glm::rotate(rot, vertices[v1.position - 1]),
            glm::rotate(rot, vertices[v2.position - 1]),
            glm::rotate(rot, vertices[v3.position - 1]),
            glm::rotate(rot, normals[v1.normal - 1]),
            glm::rotate(rot, normals[v2.normal - 1]),
            glm::rotate(rot, normals[v3.normal - 1])
        );
    }

    size_t triCount = triangles.size();
    m_data = mtl::fixed_bvh<TrianglePayload>(std::move(triangles));

    mtl::bvh_build_settings settings{ };
    settings.maxDepth = 15;
    settings.minPayloadsPerNode = 2;
    settings.split_method = mtl::bvh_split_method::OPTIMAL_SAH;
    mtl::bvh_build_stats stats{ };
    {
        std::string format = std::format("BVH build took %$ for {} triangles.", triCount);
        format.replace(format.find_first_of('%'), 2, "{}");

        sys::timer<sys::milliseconds> timer(format.c_str());
        m_data.build(&settings, &stats);
    }

    JCLOG_INFO(*g_singleThreadedLog, "Build Stats:", "");
    JCLOG_INFO(*g_singleThreadedLog, "\tTriangle Count: {}", triCount);
    JCLOG_INFO(*g_singleThreadedLog, "\tMax Depth: {}", stats.max_depth);
    JCLOG_INFO(*g_singleThreadedLog, "\tNode Count: {}", stats.node_count);
    JCLOG_INFO(*g_singleThreadedLog, "\tMax Primitives in Node: {}", stats.max_primitives_in_node);
    JCLOG_INFO(*g_singleThreadedLog, "\tMin Primitives in Node: {}", stats.min_primitives_in_node);
    JCLOG_INFO(*g_singleThreadedLog, "\tAverage Primitives per Node: {}", static_cast<float>(triCount) / stats.leaf_node_count);
    JCLOG_INFO(*g_singleThreadedLog, "\tLeaf Node Count: {}", stats.leaf_node_count);
}

glm::vec3 Blas::traverse(const mtl::ray& ray) const
{
    bool sucess = false;

    mtl::bvh_traverse_options options{ };

    mtl::bvh_traverse_stats stats{ };
    mtl::bvh_traverse_output output = m_data.traverse(ray, options, &stats);

    if( !output.payload_hit )
    {
        if( mode == 1 )
            return glm::vec3(0.f, 0.f, 0.f);
    }

    glm::vec3 sun = glm::normalize(glm::vec3{ 0.4f, -1.f, -.2f });
    float strength = 0.f;

    if( mode == 1 )
    {
        glm::vec3 hitNormal = output.payload_hit->sample_normals(ray.position + (ray.direction * output.distance));
        strength = glm::clamp(glm::dot(sun, hitNormal), 0.2f, 1.f);
    }

    switch( mode )
    {
    case 1:
        return output.payload_hit->get_color() * strength;
    case 2:
        return glm::vec3(stats.nodes_checked / 75.f);
    case 3:
        return glm::vec3(stats.primitives_checked / 30.f);
    }
    
    // dai hen
    return glm::vec3(1.f, 0.f, 0.f);
}