#include "SceneManager.h"

namespace cge
{

Scene* get_active_scene()
{
    if( SceneManager::instance().has_active_scene() )
        return SceneManager::instance().get_active_scene();
    return nullptr;
}

void set_active_scene(const std::string_view& name)
{
    SceneManager::instance().set_active_scene(name);
}

Scene* get_loaded_scene(const std::string_view& name)
{
    return SceneManager::instance().find_scene(name);
}

Scene& load_empty_scene(const std::string_view& name)
{
    return SceneManager::instance().create_empty_scene(name);
}

bool is_scene_loaded(const std::string_view& name)
{
    return SceneManager::instance().find_scene(name) != nullptr;
}

SceneManager::~SceneManager()
{
    for( Scene* scene : m_loadedScenes )
    {
        delete scene;
    }
}

Scene* SceneManager::find_scene(const std::string_view& name) const
{
    if( m_activeScene->get_name() == name )
        return m_activeScene;

    for( Scene* scene : m_loadedScenes )
    {
        if( scene->get_name() == name )
            return scene;
    }

    return nullptr;
}

Scene& SceneManager::create_empty_scene(const std::string_view& name)
{
    if( Scene* scene = find_scene(name) )
        return *scene;

    Scene* scene = new Scene(m_context, name);
    m_loadedScenes.push_back(scene);

    return *scene;
}

void SceneManager::unload_scene(const std::string_view& name)
{
    for( auto it = m_loadedScenes.begin(); it != m_loadedScenes.end(); it++ )
    {
        if( (*it)->get_name() == name )
        {
            delete (*it);
            m_loadedScenes.erase(it);
            return;
        }
    }
}

bool SceneManager::has_active_scene() const
{
    return m_activeScene != nullptr;
}

Scene* SceneManager::get_active_scene() const
{
    if( m_activeScene )
        return m_activeScene;
    return nullptr;
}

void SceneManager::set_active_scene(const std::string_view& name)
{
    if( Scene* scene = find_scene(name) )
    {
        m_activeScene = scene;
    }
}

void SceneManager::initialize(vk::RenderContext* context)
{
    if( !s_instance )
    {
        s_instance = new SceneManager(context);
    }
}

SceneManager& SceneManager::instance()
{
    return *s_instance;
}

void SceneManager::destroy()
{
    if( s_instance )
        delete s_instance;
    s_instance = nullptr;
}

} // cge