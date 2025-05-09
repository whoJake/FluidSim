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
    };
    DistributionTechnique m_distributeTechnique{ DistributionTechnique::Grid };
    void distribute_nodes();
    void distribute_nodes_debug();

    void distribute_nodes_grid();
    void distribute_nodes_circular();
    void distribute_nodes_point();

    u32 m_nodeCount{ 256 };
    f32 m_nodeRadius{ 0.25f };

    f32 m_smoothingRadius{ 5.f };
    bool m_boundryBounce{ true };
    f32 m_dampeningFactor{ 0.8f };

    f32 m_dngSpacing{ 0.75f };
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

    f32 m_gravityValue{ 9.81f };
    Viewport2D m_viewport;

    f32 m_moveSensitivity{ 1000.f };
    f32 m_zoomSensitivity{ 50.f };

    // Rendering structures
    gfx::program* m_visualiseProgram;
    gfx::descriptor_pool m_descriptorPool;
    gfx::descriptor_table* m_programTable[GFX_RI_FRAMES_IN_FLIGHT];

    gfx::buffer* m_viewportBuffers[GFX_RI_FRAMES_IN_FLIGHT];
    gfx::buffer* m_positionsBuffers[GFX_RI_FRAMES_IN_FLIGHT];
    gfx::buffer* m_nodeBuffers[GFX_RI_FRAMES_IN_FLIGHT];
};