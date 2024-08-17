#include "Entity.h"

#include <random>

bool EntityId::is_valid() const
{
    return value == invalid_id;
}

Entity::Entity(EntityId id, const EntityDef& definition) :
    m_id(id),
    m_transform(definition.position, definition.scale, glm::quat(definition.rotation)),
    m_components()
{ }

mtl::transform& Entity::transform()
{
    return m_transform;
}

const mtl::transform& Entity::transform() const
{
    return m_transform;
}

const EntityId& Entity::get_id() const
{
    return m_id;
}

bool Entity::is_valid() const
{
    return m_id.is_valid();
}