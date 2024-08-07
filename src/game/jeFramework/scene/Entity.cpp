#include "Entity.h"

namespace fw
{

Entity::Entity(EntityDef* def, u64 id) :
	m_name(def->name),
	m_id(id),
	m_blueprint(),
	m_transform(def->position, def->scale, def->rotation),
	m_flags(def->flags)
{ }

void Entity::set_blueprint(mtl::hash_string blueprint)
{
	m_blueprint = blueprint;
}

const mtl::hash_string& Entity::get_blueprint() const
{
	return m_blueprint;
}

} // fw