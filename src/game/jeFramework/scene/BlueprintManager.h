#pragma once

#include "Blueprint.h"

namespace fw
{

class BlueprintManager
{
public:
	static int initialise();
	static void shutdown();

	static Blueprint* find_blueprint(mtl::hash_string name);
	static Blueprint* create_blueprint(BlueprintDef* def);
	static void destroy_blueprint(mtl::hash_string name);
private:
	static BlueprintManager* sm_instance;

private:
	std::unordered_map<u64, Blueprint*> m_storage;
};

} // fw