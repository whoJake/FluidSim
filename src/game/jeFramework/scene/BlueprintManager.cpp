#include "BlueprintManager.h"

namespace fw
{

#define CHECK_INSTANCE(no_instance) if(!sm_instance) no_instance;
#define INSTANCE (*sm_instance)

BlueprintManager* BlueprintManager::sm_instance = nullptr;

int BlueprintManager::initialise()
{
	if( sm_instance )
	{
		return 1;
	}

	sm_instance = new BlueprintManager();
	return 0;
}

void BlueprintManager::shutdown()
{
	if( sm_instance )
	{
		delete sm_instance;
	}
}

Blueprint* BlueprintManager::find_blueprint(mtl::hash_string name)
{
	CHECK_INSTANCE(return nullptr);

	auto it = INSTANCE.m_storage.find(name.get_hash());
	if( it != INSTANCE.m_storage.end() )
	{
		return it->second;
	}

	return nullptr;
}

Blueprint* BlueprintManager::create_blueprint(BlueprintDef* def)
{
	CHECK_INSTANCE(return nullptr);

	auto it = INSTANCE.m_storage.find(def->name.get_hash());
	if( it != INSTANCE.m_storage.end() )
	{
		// maybe error? blueprint already exists.
		return it->second;
	}

	Blueprint* blueprint = new Blueprint(def);
	return INSTANCE.m_storage.emplace(def->name.get_hash(), blueprint).first->second;
}

void BlueprintManager::destroy_blueprint(mtl::hash_string name)
{
	CHECK_INSTANCE(return);

	INSTANCE.m_storage.erase(name.get_hash());
}

} // fw