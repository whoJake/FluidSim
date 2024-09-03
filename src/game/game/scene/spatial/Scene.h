#pragma once

#include "Entity.h"
#include <random>

class Scene
{
public:
    Scene();
    ~Scene();

    void pre_update();

    Entity* add_entity(const EntityDef& definition);

    void remove_entity(EntityId id);

    const std::vector<Entity*>& get_all_entities() const;
private:
    std::mt19937_64 m_randomSource;

    // begin with just a vector of entities.
    // swap out for bvh when ready
    std::vector<Entity*> m_entities;

    std::vector<EntityId> m_deletedEntities;
};