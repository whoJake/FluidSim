#include "Scene.h"

#include "imgui.h"

Scene::Scene() :
    m_randomSource()
{ }

Scene::~Scene()
{
    for( Entity* entity : m_entities )
    {
        delete entity;
    }
}

void Scene::pre_update()
{
    for( EntityId id : m_deletedEntities )
    {
        for( auto it = m_entities.begin(); it != m_entities.end(); ++it )
        {
            if( (*it)->get_id().value == id.value )
            {
                delete (*it);
                m_entities.erase(it);
                break;
            }
        }
    }

    m_deletedEntities.clear();
}

Entity* Scene::add_entity(const EntityDef& definition)
{
    EntityId newId = EntityId::generate(m_randomSource);
    Entity* result = new Entity(newId, definition);
    m_entities.push_back(result);
    return result;
}

void Scene::remove_entity(EntityId id)
{
    m_deletedEntities.push_back(id);
}

const std::vector<Entity*>& Scene::get_all_entities() const
{
    return m_entities;
}

void Scene::draw_debug_panel()
{
    ImGui::Begin("Scene");
    for( Entity* entity : m_entities )
    {
        glm::vec3 pos = entity->transform().get_position();
        ImGui::DragFloat3("Position", &pos.x);
    }
    ImGui::End();
}

void Scene::for_each(std::function<void(Entity*)> func)
{
    for( Entity* entity : m_entities )
    {
        func(entity);
    }
}