#include "Blueprint.h"
#include "Entity.h"

namespace fw
{

Blueprint::Blueprint(BlueprintDef* def) :
	m_name(def->name),
	m_mesh(),
	m_material(def->material)
{ }

Entity* Blueprint::create_entity(EntityDef* def, u64 id)
{
	Entity* retval = new Entity(def, id);
	retval->set_blueprint(m_name);
	return retval;
}

const mtl::hash_string& Blueprint::get_name() const
{
	return m_name;
}

mtl::mesh& Blueprint::get_mesh()
{
	return m_mesh;
}

gfx::MaterialDefinition& Blueprint::get_material()
{
	return m_material;
}

} // fw