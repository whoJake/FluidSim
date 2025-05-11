#pragma once

#include "base/game.h"
#include "platform/events/WindowEvent.h"
#include "implementations/ImGuiContext.h"
#include "fluidsim/FluidSim2D.h"

#include "Viewport2D.h"

#include "gfx_fw/render_interface.h"
#include "gfx_core/shader.h"

class FluidApp : public fw::game
{
public:
    FluidApp() = default;
    ~FluidApp() = default;

    void setup_startup_graph(fw::scaffold_startup_node& parent) override;
    void setup_update_graph(fw::scaffold_update_node& parent) override;
    void setup_shutdown_graph(fw::scaffold_shutdown_node& parent) override;

    void on_event(Event& e) override;

    fw::window::state get_window_startup_state() override;
private:
    void initialise_app();
    void shutdown_app();

    void initialise_simulation();
    void update_simulation();
    void update_simulation_debug();
    void shutdown_simulation();

    void render_simulation();
    void render_simulation_debug();

    void update_simulation_buffers();

    void update_movement();

    enum class DistributionTechnique
    {
        Grid = 0,
        Circular,
        Point,
        Random,
    };
    DistributionTechnique m_distributeTechnique{ DistributionTechnique::Random };
    void distribute_nodes();
    void distribute_nodes_debug();

    void distribute_nodes_grid();
    void distribute_nodes_circular();
    void distribute_nodes_point();
    void distribute_nodes_random();

    u32 m_nodeCount{ 576 };
    f32 m_nodeRadius{ 0.25f };

    f32 m_smoothingRadius{ 2.5f };
    bool m_boundryBounce{ true };
    f32 m_dampeningFactor{ 0.8f };
    f32 m_targetDensity{ 3.f };
    f32 m_pressureMultiplier{ 0.9f };

    f32 m_dngSpacing{ 0.30f };
    void distribute_nodes_grid_debug();

    f32 m_dncVelocityScale{ 5.f };
    f32 m_dncRadius{ 1.f };
    void distribute_nodes_circular_debug();

    f32 m_dnpVelocityScale{ 5.f };
    void distribute_nodes_point_debug();

    bool on_window_resize(WindowResizeEvent& e);
    bool on_window_closed(WindowClosedEvent& e);
private:
    std::unique_ptr<mygui::Context> m_imGui;
    std::unique_ptr<FluidSim2D> m_simulation;

    // Settings
    bool m_simPaused{ true };
    f32 m_simWidth{ 20.f };
    f32 m_simHeight{ 20.f };

    f32 m_gravityValue{ 0.f };
    Viewport2D m_viewport;

    f32 m_moveSensitivity{ 1000.f };
    f32 m_zoomSensitivity{ 50.f };

    enum class VisualiseType
    {
        FlatColor = 0,
        MouseSelection,
        DensityView,
    };
    VisualiseType m_visualiseType{ VisualiseType::DensityView };
    glm::f32vec3 m_nodeColor{ 1.f, 1.f, 1.f };
    glm::f32vec3 m_paintColor{ 1.f, 0.f, 0.f };
    f32 m_paintRadius{ 1.5f };
    f32 m_minDensityDisplay{ 0.f };
    f32 m_maxDensityDisplay{ 6.f };
    glm::f32vec3 m_densityMinColor{ 0.f, 0.f, 1.f };
    glm::f32vec3 m_densityMaxColor{ 1.f, 0.f, 0.f };

    glm::f32vec2 m_mouseWorldPosition{ };

    // Rendering structures
    gfx::program* m_visualiseProgram;
    gfx::descriptor_pool m_descriptorPool;
    gfx::descriptor_table* m_programTable[GFX_RI_FRAMES_IN_FLIGHT];

    gfx::buffer* m_viewportBuffers[GFX_RI_FRAMES_IN_FLIGHT];
    gfx::buffer* m_positionsBuffers[GFX_RI_FRAMES_IN_FLIGHT];
    gfx::buffer* m_nodeBuffers[GFX_RI_FRAMES_IN_FLIGHT];
};