#pragma once

#include "data/spatial.h"
#include "Mesh.h"

typedef uint32_t bpid_t;

class Blueprint
{
public:
    Blueprint(const std::string_view& name = "", uint32_t vertexBufferCount = 1);
    ~Blueprint();
    Blueprint(Blueprint&&);
    Blueprint(const Blueprint&) = default;

    Blueprint& operator=(Blueprint&&) = default;
    Blueprint& operator=(const Blueprint&) = default;

    const std::string_view& get_name() const;

    AABoundingBox<> get_bounds() const;

    Mesh& mesh();

    bpid_t get_id() const;
private:
    std::string_view m_name;
    AABoundingBox<> m_boundingBox{ };
    std::unique_ptr<Mesh> m_mesh;
    bpid_t m_id;
};