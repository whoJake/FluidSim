#pragma once
#include "data/hash_string.h"
#include "data/aabb.h"
#include "data/mesh.h"

#include "graphics/Material.h"

namespace fw
{

struct EntityDef;
class Entity;

struct BlueprintDef
{
	mtl::hash_string name;
	mtl::aabb3 extents;
	mtl::hash_string source;

	gfx::MaterialDefinition material;
};

class Blueprint
{
public:
	Blueprint(BlueprintDef* def);
	~Blueprint() = default;

	Entity* create_entity(EntityDef* def, u64 id);

	const mtl::hash_string& get_name() const;
	mtl::mesh& get_mesh();
	gfx::MaterialDefinition& get_material();
private:
	mtl::hash_string m_name;
	mtl::mesh m_mesh;
	gfx::MaterialDefinition m_material;
};

} // fw