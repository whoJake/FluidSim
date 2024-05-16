#include "Entity.h"
#include "Blueprint.h"

#include <stdlib.h>

Entity::Entity(bpid_t blueprintId,
               const glm::vec3& position,
               const glm::vec3& scale,
               const glm::vec3& rotation) :
    m_bpid(blueprintId),
    m_transform(position, scale, glm::quat(rotation))
{
    m_id = static_cast<entid_t>(std::hash<uint32_t>()(m_bpid + std::rand()));
}

Entity::~Entity()
{ }

Transform& Entity::transform()
{
    return m_transform;
}

const Transform& Entity::transform() const
{
    return m_transform;
}

entid_t Entity::get_id() const
{
    return m_id;
}

bpid_t Entity::get_bpid() const
{
    return m_bpid;
}