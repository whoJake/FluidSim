#pragma once

#include "../Component.h"
#include "data/hash_string.h"
#include "data/mesh.h"

namespace fw
{

class RenderableMeshComponent : public Component
{
public:
    RenderableMeshComponent(std::string source);
    ~RenderableMeshComponent() = default;

    DEFAULT_COPY(RenderableMeshComponent);
    DEFAULT_MOVE(RenderableMeshComponent);

    void load();
    void unload();

    const mtl::mesh& get_mesh() const;
    const mtl::hash_string& get_hash_string() const;
private:
    mtl::hash_string m_hash;
    std::string m_source;

    std::unique_ptr<mtl::mesh> m_mesh;
};

} // fw