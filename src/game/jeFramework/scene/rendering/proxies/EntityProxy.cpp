#include "EntityProxy.h"

EntityProxy::EntityProxy(Entity* entity) :
    m_entity(entity),
    m_transform(entity->transform()),
    m_blueprintProxy(entity->get_bpid()),
    m_id(entity->get_id())
{ }

void EntityProxy::sync()
{
    m_transform = m_entity->transform();
    m_blueprintProxy = m_entity->get_bpid();
}

glm::mat4 EntityProxy::get_model_matrix() const
{
    return m_transform.as_matrix();
}

entid_t EntityProxy::get_id() const
{
    return m_id;
}

bpid_t EntityProxy::get_bpid() const
{
    return m_blueprintProxy;
}