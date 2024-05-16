#pragma once

#include "scene/gameplay/Entity.h"
#include "data/spatial.h"

class EntityProxy
{
public:
    EntityProxy(Entity* entity);
    ~EntityProxy() = default;

    void sync();

    glm::mat4 get_model_matrix() const;

    entid_t get_id() const;

    bpid_t get_bpid() const;
private:
    Entity* m_entity;
    Transform m_transform;
    bpid_t m_blueprintProxy;
    entid_t m_id;
};