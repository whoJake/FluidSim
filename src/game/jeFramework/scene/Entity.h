#pragma once

#include "data/hash_string.h"
#include "data/transform.h"
#include "Blueprint.h"

namespace fw
{

struct EntityDef
{
	mtl::hash_string name;
	mtl::hash_string blueprint;

	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;
	u32 flags;
};

class Entity
{
public:
	Entity(EntityDef* def, u64 id);
	~Entity() = default;

	void set_blueprint(mtl::hash_string name);
	const mtl::hash_string& get_blueprint() const;
private:
	mtl::hash_string m_name;
	u64 m_id;
	mtl::hash_string m_blueprint;

	mtl::transform m_transform;
	u32 m_flags;
};

} // fw