#include "Scene.h"
#include "BlueprintManager.h"

#include <random>

namespace fw
{

Scene::Scene() :
	m_entities()
{ }

Scene::~Scene()
{ }

void Scene::pre_update()
{
	// delete entities in queue first.
	for( u64 id : m_deletedEntities )
	{
		// stdlib is nice enough we don't have to check if it
		// exists or not.
		m_entities.erase(id);
	}
}

Entity* Scene::create_entity(EntityDef* def)
{
	// generate an id, the same EntityDef has no problem creating 2 entities
	// this should be moved to its own provider object (determinism, multi-threading)
	std::random_device dev;
	std::mt19937_64 rng(dev());
	std::uniform_int_distribution<std::mt19937_64::result_type> dist(u64_min, u64_max);
	u64 id = dist(rng);

	Entity* entity;
	Blueprint* blueprint = BlueprintManager::find_blueprint(def->blueprint);
	if( blueprint )
	{
		entity = blueprint->create_entity(def, id);
	}
	else
	{
		// just create the entity anyway. fuck it
		entity = new Entity(def, id);
	}

	auto pair = m_entities.emplace(id, entity);
	TRAP_NEQ(pair.second, true, "Entity hash collision at: {}", id);

	return pair.first->second;
}

bool Scene::destroy_entity(u64 id)
{
	auto it = m_entities.find(id);
	if( it != m_entities.end() )
	{
		m_deletedEntities.push_back(id);
		return true;
	}

	return false;
}

std::vector<Entity*> Scene::get_all_entities() const
{
	std::vector<Entity*> retval;
	retval.reserve(m_entities.size());

	for( const auto& pair : m_entities )
	{
		retval.push_back(pair.second);
	}

	return retval;
}

} // fw