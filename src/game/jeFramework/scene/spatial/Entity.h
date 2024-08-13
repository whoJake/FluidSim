#pragma once

#include "Component.h"
#include "data/transform.h"
#include <random>

namespace fw
{

struct EntityId
{
    inline static constexpr u64 invalid_id = 0;
    u64 value;

    bool is_valid() const;

    static EntityId generate(std::mt19937_64& source)
    {
        static std::uniform_int_distribution<std::mt19937_64::result_type> distribution(1u, u64_max);
        return { distribution(source) };
    }
};

struct EntityDef
{
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
};

class Entity
{
public:
    Entity() = default;
    Entity(EntityId id, const EntityDef& definition);
    ~Entity() = default;

    DEFAULT_COPY(Entity);
    DEFAULT_MOVE(Entity);

    template<typename ComponentT>
    bool has_component() const;

    template<typename ComponentT>
    ComponentT* get_component();

    template<typename ComponentT>
    const ComponentT* get_component() const;

    template<typename ComponentT, typename... Args>
    ComponentT* add_component(Args&&... forwarded_args);

    template<typename ComponentT>
    void remove_component();

    mtl::transform& transform();
    const mtl::transform& transform() const;

    const EntityId& get_id() const;

    bool is_valid() const;
private:
    using ComponentId = u64;
    
    template<typename ComponentT>
    static ComponentId get_component_id();
private:
    EntityId m_id{ EntityId::invalid_id };
    mtl::transform m_transform{ };

    std::unordered_map<ComponentId, u64> m_componentMapping;
    std::vector<std::unique_ptr<Component>> m_components;
};

} // fw

#ifndef INC_FW_ENTITY_INL
#define INC_FW_ENTITY_INL
#include "Entity.inl"
#endif