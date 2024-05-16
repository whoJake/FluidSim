#pragma once

#include "Blueprint.h"
#include "data/spatial.h"

class Blueprint;

typedef uint32_t entid_t;

class Entity
{
public:
    Entity(bpid_t blueprintId = 0,
           const glm::vec3& position = { 0.f, 0.f, 0.f },
           const glm::vec3& scale    = { 1.f, 1.f, 1.f },
           const glm::vec3& rotation = { 0.f, 0.f, 0.f });
    ~Entity();

    Entity(Entity&&) = default;
    Entity(const Entity&) = default;
    Entity& operator=(Entity&&) = default;
    Entity& operator=(const Entity&) = default;

    Transform& transform();
    const Transform& transform() const;

    entid_t get_id() const;
    bpid_t get_bpid() const;
private:
    Transform m_transform;
    bpid_t m_bpid;
    entid_t m_id;
};