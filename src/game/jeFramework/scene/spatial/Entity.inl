#include "Entity.h"

namespace fw
{

#define STATIC_CHECK_COMPONENT_T(type) static_assert(std::is_base_of_v<Component, type>)

template<typename ComponentT>
Entity::ComponentId Entity::get_component_id()
{
    STATIC_CHECK_COMPONENT_T(ComponentT);

    return static_cast<ComponentId>(typeid(ComponentT).hash_code());
}

template<typename ComponentT>
bool Entity::has_component() const
{
    STATIC_CHECK_COMPONENT_T(ComponentT);

    ComponentId id = get_component_id<ComponentT>();
    return m_componentMapping.find(id) != m_componentMapping.end();
}

template<typename ComponentT>
ComponentT* Entity::get_component()
{
    STATIC_CHECK_COMPONENT_T(ComponentT);

    ComponentId id = get_component_id<ComponentT>();
    auto it = m_componentMapping.find(id);
    if( it != m_componentMapping.end() )
    {
        return static_cast<ComponentT*>(m_components[it->second].get());
    }

    return nullptr;
}

template<typename ComponentT>
const ComponentT* Entity::get_component() const
{
    STATIC_CHECK_COMPONENT_T(ComponentT);

    ComponentId id = get_component_id<ComponentT>();
    auto it = m_componentMapping.find(id);
    if( it != m_componentMapping.end() )
    {
        return static_cast<ComponentT*>(m_components[it->second].get());
    }

    return nullptr;
}

template<typename ComponentT, typename... Args>
ComponentT* Entity::add_component(Args&&... forwardedArgs)
{
    STATIC_CHECK_COMPONENT_T(ComponentT);

    if( has_component<ComponentT>() )
    {
        nullptr;
    }

    m_components.push_back(std::make_unique<ComponentT>(std::forward<Args>(forwardedArgs)...));

    ComponentId id = get_component_id<ComponentT>();
    m_componentMapping.emplace(
        std::piecewise_construct,
        std::tuple(id),
        std::tuple(m_components.size() - 1u));

    return get_component<ComponentT>();
}

template<typename ComponentT>
void Entity::remove_component()
{
    STATIC_CHECK_COMPONENT_T(ComponentT);

    if( !has_component<ComponentT>() )
    {
        return;
    }

    ComponentId id = get_component_id<ComponentT>();
    u64 idx = m_componentMapping[id];

    std::swap(m_components[idx], m_components.back());
    u64 swapidx = m_components.size() - 1u;

    for( auto& [componentId, componentIdx] : m_componentMapping )
    {
        if( componentIdx == swapidx )
        {
            componentIdx = idx;
            break;
        }
    }

    m_components.pop_back();
    m_componentMapping.erase(id);
}

} // fw