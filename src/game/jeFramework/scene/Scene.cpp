#include "Scene.h"

Scene::Scene(vk::RenderContext* context, const std::string_view& name) :
    m_name(name),
    m_context(context)
{ }

const std::string_view& Scene::get_name() const
{
    return m_name;
}

std::unordered_map<entid_t, Entity>& Scene::get_scene_entities()
{
    return m_entities;
}

Entity* Scene::request_create_entity(Entity&& entity)
{
    std::lock_guard<std::mutex> lock(m_entityCreationMutex);
    m_entityCreationQueue.push(std::move(entity));
    return &m_entityCreationQueue.back();
}

void Scene::request_destroy_entity(entid_t entity)
{
    std::lock_guard<std::mutex> lock(m_entityDestructionMutex);
    m_entityDestructionQueue.push(entity);
}

Blueprint* Scene::request_create_blueprint(Blueprint&& blueprint)
{
    std::lock_guard<std::mutex> lock(m_blueprintCreationMutex);
    m_blueprintCreationQueue.push(std::move(blueprint));
    return &m_blueprintCreationQueue.back();
}

void Scene::request_destroy_blueprint(bpid_t blueprint)
{
    std::lock_guard<std::mutex> lock(m_blueprintDestructionMutex);
    m_blueprintDestructionQueue.push(blueprint);
}

Blueprint* Scene::get_blueprint(bpid_t id)
{
    auto it = m_blueprints.find(id);
    if( it != m_blueprints.end() )
    {
        return &(it->second);
    }

    return nullptr;
}

Entity* Scene::get_entity(entid_t id)
{
    auto it = m_entities.find(id);
    if( it != m_entities.end() )
    {
        return &(it->second);
    }

    return nullptr;
}

const std::unordered_map<bpid_t, BlueprintProxy>& Scene::get_blueprint_proxies() const
{
    return m_blueprintProxies;
}

const std::unordered_map<bpid_t, EntityProxy>& Scene::get_entity_proxies() const
{
    return m_entityProxies;
}

void Scene::sync_proxies()
{ 
    // Maybe setup as task?

    for( auto& entity : m_entities )
    {
        auto entit = m_entityProxies.find(entity.second.get_id());
        auto bpit = m_blueprintProxies.find(entity.second.get_bpid());

        if( entit != m_entityProxies.end() )
        {
            entit->second.sync();
        }

        // Only sync blueprints that are actually being used?
        if( bpit != m_blueprintProxies.end() )
        {
            auto it = m_blueprints.find(entity.second.get_bpid());

            if( it != m_blueprints.end() )
            {
                bpit->second.sync();
            }
        }
    }
}

void Scene::resolve_creation_queue()
{
    // Create blueprints
    {
        std::lock_guard<std::mutex> lock(m_blueprintCreationMutex);
        for( ; !m_blueprintCreationQueue.empty(); m_blueprintCreationQueue.pop() )
        {
            bpid_t insertId = m_blueprintCreationQueue.front().get_id();
            Blueprint& insertedBlueprint = m_blueprints.emplace(std::piecewise_construct,
                                                                std::tuple(insertId),
                                                                std::tuple(std::move(m_blueprintCreationQueue.front()))).first->second;

            m_blueprintProxies.emplace(std::piecewise_construct,
                                       std::tuple(insertId),
                                       std::tuple(m_context, &insertedBlueprint));
        }
    }

    // Create entities
    {
        std::lock_guard<std::mutex> lock(m_entityCreationMutex);
        for( ; !m_entityCreationQueue.empty(); m_entityCreationQueue.pop() )
        {
            entid_t insertId = m_entityCreationQueue.front().get_id();
            Entity& insertedEntity = m_entities.emplace(std::piecewise_construct,
                                                        std::tuple(insertId),
                                                        std::tuple(std::move(m_entityCreationQueue.front()))).first->second;

            m_entityProxies.emplace(std::piecewise_construct,
                                    std::tuple(insertId),
                                    std::tuple(&insertedEntity));
        }
    }
}

void Scene::resolve_destruction_queue()
{
    // Destroy blueprints
    {
        std::lock_guard<std::mutex> lock(m_blueprintDestructionMutex);
        for( ; !m_blueprintDestructionQueue.empty(); m_blueprintDestructionQueue.pop() )
        {
            auto it = m_blueprints.find(m_blueprintDestructionQueue.front());
            if( it != m_blueprints.end() )
            {
                m_blueprints.erase(it);
            }

            auto it2 = m_blueprintProxies.find(m_blueprintDestructionQueue.front());
            if( it2 != m_blueprintProxies.end() )
            {
                m_blueprintProxies.erase(it2);
            }
        }
    }

    // Destroy entities
    {
        std::lock_guard<std::mutex> lock(m_entityDestructionMutex);
        for( ; !m_entityDestructionQueue.empty(); m_entityDestructionQueue.pop() )
        {
            auto it = m_entities.find(m_entityDestructionQueue.front());
            if( it != m_entities.end() )
            {
                m_entities.erase(it);
            }

            // Destroy associating proxy
            auto it2 = m_entityProxies.find(m_entityDestructionQueue.front());
            if( it2 != m_entityProxies.end() )
            {
                m_entityProxies.erase(it2);
            }
        }
    }
}