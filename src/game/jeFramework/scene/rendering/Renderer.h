#pragma once

#include "rendering/RenderContext.h"
#include "proxies/BlueprintProxy.h"
#include "proxies/EntityProxy.h"
#include "proxies/MeshProxy.h"
#include "scene/gameplay/Camera.h"

struct SceneProxies
{
    const std::unordered_map<bpid_t, BlueprintProxy>& blueprints;
    const std::unordered_map<entid_t, EntityProxy>& entities;
};

struct DebugMaterial
{
    std::unique_ptr<vk::RenderPass> renderPass{ nullptr };
    std::unique_ptr<vk::PipelineLayout> pipelineLayout{ nullptr };
    std::unique_ptr<vk::Pipeline> pipeline{ nullptr };
    vk::PipelineState pipelineState{ };
};

PARAM(wireframe);
PARAM(disable_backface_culling);

class Renderer
{
public:
    Renderer(vk::RenderContext& context);
    ~Renderer();

    inline const vk::RenderPass& get_render_pass() const
    {
        return *m_debugMaterial.renderPass;
    }

    void dispatch_render(SceneProxies scene, const std::vector<Camera*>& cameras, uint32_t* fence);
private:
    void build_debug_material();
private:
    vk::RenderContext& m_context;
    DebugMaterial m_debugMaterial{ };

    struct CameraMatrixData
    {
        glm::mat4 projection;
        glm::mat4 view;
    };
};