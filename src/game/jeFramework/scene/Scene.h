#pragma once

#include "threading/threading.h"

#include "gameplay/Entity.h"
#include "gameplay/Blueprint.h"
#include "rendering/proxies/EntityProxy.h"
#include "rendering/proxies/BlueprintProxy.h"
#include "rendering/RenderContext.h"

class Scene
{
public:
    Scene(vk::RenderContext* context, const std::string_view& name);
    ~Scene() = default;

    Scene(Scene&&) = default;
    Scene(const Scene&) = default;

    Scene& operator=(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;

    const std::string_view& get_name() const;

    std::unordered_map<entid_t, Entity>& get_scene_entities();

    Entity* request_create_entity(Entity&& entity);
    void request_destroy_entity(entid_t entity);

    Blueprint* request_create_blueprint(Blueprint&& blueprint);
    void request_destroy_blueprint(bpid_t blueprint);

    Blueprint* get_blueprint(bpid_t id);
    Entity* get_entity(entid_t id);

    const std::unordered_map<bpid_t, BlueprintProxy>& get_blueprint_proxies() const;
    const std::unordered_map<entid_t, EntityProxy>& get_entity_proxies() const;

    void sync_proxies();

    void resolve_creation_queue();
    void resolve_destruction_queue();
private:
    vk::RenderContext* m_context;
    std::string_view m_name;

    std::unordered_map<bpid_t, Blueprint> m_blueprints;
    std::unordered_map<entid_t, Entity> m_entities;

    std::unordered_map<bpid_t, BlueprintProxy> m_blueprintProxies;
    std::unordered_map<entid_t, EntityProxy> m_entityProxies;

    std::queue<Entity> m_entityCreationQueue;
    std::mutex m_entityCreationMutex;

    std::queue<entid_t> m_entityDestructionQueue;
    std::mutex m_entityDestructionMutex;

    std::queue<Blueprint> m_blueprintCreationQueue;
    std::mutex m_blueprintCreationMutex;

    std::queue<bpid_t> m_blueprintDestructionQueue;
    std::mutex m_blueprintDestructionMutex;
};