#pragma once

#include "Entity.h"

namespace fw
{

/// <summary>
/// Scene holds only the information about the layout of objects. No attempt is
/// made to load/unload assets.
/// </summary>
class Scene
{
public:
	Scene();
	~Scene();

	void pre_update();

	Entity* create_entity(EntityDef* def);
	
	/// <summary>
	/// Marks an entity with a given ID to be destroyed next frame.
	/// </summary>
	/// <returns>Whether the entity existed.</returns>
	bool destroy_entity(u64 id);

	std::vector<Entity*> get_all_entities() const;
private:
	// temporary, just put them in a map
	std::unordered_map<u64, Entity*> m_entities;

	std::vector<u64> m_deletedEntities;
};

} // fw