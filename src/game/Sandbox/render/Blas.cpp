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
    // device.open("assets/obj/12140_Skull_v3_L2.obj");
    device.open("assets/obj/apple.obj");

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

        glm::vec3 rotation{ 0.f, 0.f, 0.f };
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

    SYSLOG_INFO("Build Stats:", "");
    SYSLOG_INFO("\tTriangle Count: {}", triCount);
    SYSLOG_INFO("\tMax Depth: {}", stats.max_depth);
    SYSLOG_INFO("\tNode Count: {}", stats.node_count);
    SYSLOG_INFO("\tMax Primitives in Node: {}", stats.max_primitives_in_node);
    SYSLOG_INFO("\tMin Primitives in Node: {}", stats.min_primitives_in_node);
    SYSLOG_INFO("\tAverage Primitives per Node: {}", static_cast<float>(triCount) / stats.leaf_node_count);
    SYSLOG_INFO("\tLeaf Node Count: {}", stats.leaf_node_count);
    SYSLOG_INFO("\tPrimitive Count: {}", m_data.count_primitives());
}

glm::vec3 Blas::traverse(const mtl::ray& ray, const glm::vec3& sun, u32 bounces) const
{
    bool sucess = false;

    mtl::bvh_traverse_options options{ };

    mtl::ray cRay = ray;
    glm::vec3 color{ 0.f, 0.f, 0.f };
    float firstDist = 0.f;
    bool hasColor = false;
    mtl::bvh_traverse_stats stats{ };
    for( u32 iteration = 0; iteration <= bounces; iteration++ )
    {
        mtl::bvh_traverse_output result = m_data.traverse(cRay, options, &stats);
        if( !result.payload_hit )
        {
            // no hit. we're finished here.
            break;
        }

        glm::vec3 hitNormal = result.payload_hit->sample_normals(cRay.position + (cRay.direction * result.distance));
        glm::vec3 hitColor = result.payload_hit->get_color() * std::clamp(glm::dot(sun, hitNormal), 0.2f, 1.f);

        if( hasColor )
        {
            color = (color + hitColor) / 2.f;
        }
        else
        {
            color = hitColor;
            firstDist = result.distance;
            hasColor = true;
        }

        cRay.position += cRay.direction * (result.distance - 0.00001f);
        cRay.direction = glm::reflect(cRay.direction, hitNormal);
        cRay.invDirection = glm::vec3(
            1.f / cRay.direction.x,
            1.f / cRay.direction.y,
            1.f / cRay.direction.z);
    }

    switch( mode )
    {
    case 1:
        return color;
    case 2:
        return glm::vec3(stats.nodes_checked / 75.f);
    case 3:
        return glm::vec3(stats.primitives_checked / 30.f);
    case 4:
        return glm::vec3(firstDist / 20.f);
    }
    
    // dai hen
    return glm::vec3(1.f, 0.f, 0.f);
}