#pragma once

#include "scene/gameplay/Blueprint.h"
#include "scene/gameplay/Mesh.h"
#include "MeshProxy.h"

class BlueprintProxy
{
public:
    BlueprintProxy(vk::RenderContext* context, Blueprint* blueprint);
    ~BlueprintProxy() = default;

    void sync();

    bpid_t get_id() const;

    const MeshProxy& get_mesh_proxy() const;
private:
    Blueprint* m_blueprint;
    uint32_t m_materialProxy;
    MeshProxy m_meshProxy;
};