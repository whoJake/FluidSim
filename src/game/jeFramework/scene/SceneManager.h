#pragma once

#include "Scene.h"

namespace cge
{

Scene* get_active_scene();
void set_active_scene(const std::string_view& name);

Scene* get_loaded_scene(const std::string_view& name);

Scene& load_empty_scene(const std::string_view& name);
bool is_scene_loaded(const std::string_view& name);

class SceneManager
{
public:
    ~SceneManager();

    SceneManager(SceneManager&&) = delete;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    Scene* find_scene(const std::string_view& name) const;

    Scene& create_empty_scene(const std::string_view& name);

    void unload_scene(const std::string_view& name);

    bool has_active_scene() const;

    Scene* get_active_scene() const;

    void set_active_scene(const std::string_view& name);
private:
    SceneManager(vk::RenderContext* context) :
        m_context(context)
    { }
private:
    vk::RenderContext* m_context;
    std::vector<Scene*> m_loadedScenes{ };
    Scene* m_activeScene{ nullptr };
public:
    static void initialize(vk::RenderContext* context);
    static SceneManager& instance();
    static void destroy();
private:
    inline static SceneManager* s_instance{ nullptr };
};

} // cge