#pragma once

#include "data/fixed_vector.h"
#include "data/hash_string.h"
#include "RenderData.h"

#include "scene/spatial/Scene.h"
#include "scene/spatial/Entity.h"
#include "scene/spatial/components/RenderableMesh.h"

#include "core/BufferView.h"
#include "graphics/UniformBuffer.h"
#include "graphics/streaming/Streamer.h"

#include "graphics/DrawData.h"
#include "implementations/ImGuiContext.h"

#define CFG_ENABLE_IMGUI 1

// Forward declares
namespace vk
{
class Buffer;

class RenderContext;
class RenderPass;
} // vk

namespace fw
{
class window;

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
    ~Renderer();

#if CFG_ENABLE_IMGUI
    void init_debug_imgui(fw::window* window);
#endif

    void pre_render(Scene* scene, float deltaTime);

    void render();

    void restart();
public:
    static constexpr u32 max_cameras = 8;
    static constexpr u32 max_models = 5000;
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

    void load_draw_data(RenderableMeshComponent* mesh);
private:
    struct State
    {
        u32 frameIndex{ 0 };

        u32 activeCameras{ 0 };
        u32 models{ 0 };
        std::vector<fw::gfx::DrawData*> drawList;
    };
private:
    vk::RenderContext& m_context;
    fw::gfx::Streamer m_streamer;

    std::unique_ptr<fw::gfx::UniformBuffer> m_frameData;
    std::unique_ptr<fw::gfx::UniformBuffer> m_cameraData;
    std::unique_ptr<fw::gfx::UniformBuffer> m_modelData;

    std::unordered_map<mtl::hash_string, std::unique_ptr<fw::gfx::DrawData>> m_meshes;
    std::unordered_map<mtl::hash_string, fw::gfx::StreamHandle*> m_streaming;

    State m_state;

    //temp
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::unique_ptr<fw::gfx::Shader> m_shader;
    std::unique_ptr<fw::gfx::Material> m_material;
#if CFG_ENABLE_IMGUI
    std::unique_ptr<mygui::Context> m_imgui;
#endif
};