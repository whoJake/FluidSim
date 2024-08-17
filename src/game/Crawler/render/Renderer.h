#pragma once

#include "data/fixed_vector.h"
#include "data/hash_string.h"
#include "RenderData.h"

#include "scene/spatial/Scene.h"
#include "scene/spatial/Entity.h"

#include "core/BufferView.h"

// Forward declares
namespace vk
{
class Buffer;

class RenderContext;
class RenderPass;
} // vk

namespace fw
{
namespace gfx
{
class Shader;
class Material;
} // gfx
} // fw

class Renderer
{
public:
    Renderer(vk::RenderContext& context);
    ~Renderer() = default;

    void pre_render(Scene* scene, float deltaTime);

    void render();

    void restart();
public:
    static constexpr u32 max_cameras = 8;
    static constexpr u32 max_models = 32;
private:
    /// <summary>
    /// Return a buffer view of the current frames FrameData.
    /// </summary>
    vk::BufferView get_frame_data_view() const;

    /// <summary>
    /// Return a buffer view of the current frames CameraData for a given camera idx.
    /// </summary>
    vk::BufferView get_camera_data_view(u32 idx) const;

    /// <summary>
    /// Return a buffer view of the current frames ModelData for a given model idx.
    /// </summary>
    vk::BufferView get_model_data_view(u32 idx) const;

    void add_camera(Entity* entity);
    void add_renderable(Entity* entity);

    void create_renderpass();
private:
    struct Drawable
    {
        mtl::hash_string material;
        u32 vertexCount;
        vk::BufferView modelBuffer;
        vk::BufferView vertexBuffer;
    };

    struct State
    {
        u32 frameIndex;

        u32 activeCameras;
        u32 models;
        std::vector<Drawable> drawList;
    };
private:
    vk::RenderContext& m_context;

    std::unique_ptr<vk::Buffer> m_frameData;
    std::unique_ptr<vk::Buffer> m_cameraData;
    std::unique_ptr<vk::Buffer> m_modelData;

    std::unordered_map<mtl::hash_string, std::unique_ptr<vk::Buffer>> m_loadedMeshes;

    State m_state;

    //temp
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::unique_ptr<fw::gfx::Shader> m_shader;
    std::unique_ptr<fw::gfx::Material> m_material;
};