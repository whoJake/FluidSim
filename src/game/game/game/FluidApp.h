#pragma once

#include "base/game.h"
#include "platform/events/WindowEvent.h"
#include "implementations/ImGuiContext.h"
#include "fluidsim/FluidSim2D.h"

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
    void initialise_simulation();
    void update_simulation();
    void update_simulation_debug();
    void shutdown_simulation();

    void render_simulation();
    void render_simulation_debug();

    void update_simulation_buffers();
    void update_simulation_settings();

    bool on_window_resize(WindowResizeEvent& e);
    bool on_window_closed(WindowClosedEvent& e);
private:
    std::unique_ptr<mygui::Context> m_imGui;
    std::unique_ptr<FluidSim2D> m_simulation;

    // Settings
    bool m_simulationPaused{ true };
    u32 m_simulationNodes{ 150 };
    f32 m_simulationWidth{ 10.f };
    f32 m_simulationHeight{ 10.f };
    f32 m_simulationNodeRadius{ 0.15f };

    f32 m_gravityValue{ 9.81f };

    // Rendering structures
    gfx::program* m_visualiseProgram;
    gfx::descriptor_pool m_descriptorPool;
    gfx::descriptor_table* m_programTable[GFX_RI_FRAMES_IN_FLIGHT];

    struct FrameInfo
    {
        glm::f32vec2 sim_to_local;
    };

    gfx::buffer* m_nodeBuffers[GFX_RI_FRAMES_IN_FLIGHT];

    gfx::buffer* m_frameInfoBuffers[GFX_RI_FRAMES_IN_FLIGHT];
    bool m_frameInfoDirty[GFX_RI_FRAMES_IN_FLIGHT];
};