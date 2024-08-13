#pragma once

#include "Entity.h"
#include <type_traits>

namespace fw
{

class ECS
{
public:
    void add_entity(EntityId id);

    void remove_entity(EntityId id);

    template<typename ComponentT>
    ComponentT& add_component(EntityId id, ComponentT&& component);

    template<typename ComponentT>
    ComponentT& add_component(EntityId id, const ComponentT& component);

    template<typename ComponentT>
    void remove_component(EntityId id);

    template<typename... ComponentT>
    std::tuple<ComponentT&...> get(EntityId id);

    template<typename... ComponentT>
    std::tuple<const ComponentT&...> get(EntityId id) const;

    template<typename... ComponentT>
    bool has(EntityId id) const;

    template<typename... ComponentT>
    void for_each(EntityId id, std::function<void(ComponentT&...)> func);

    template<typename... ComponentT>
    void for_each(EntityId id, std::function<void(const ComponentT&...)> func) const;

    template<typename... ComponentT>
    void for_each(EntityId id, std::function<void(EntityId, ComponentT&...)> func);

    template<typename... ComponentT>
    void for_each(EntityId id, std::function<void(EntityId, const ComponentT&...)> func) const;
private:
};

} // fw