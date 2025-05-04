#include "FluidApp.h"

#include "input/imgui/imgui_bindings.h"
#include "input/Input.h"
#include "basic/Time.h"

#include "gfx_core/driver.h"
#include "gfx_fw/program_mgr.h"

void FluidApp::on_event(Event& e)
{
    Input::register_event(e);
    mygui::dispatch_event(e);

    EventDispatcher dispatch(e);
    dispatch.dispatch<WindowResizeEvent>(BIND_EVENT_FN(FluidApp::on_window_resize));
    dispatch.dispatch<WindowClosedEvent>(BIND_EVENT_FN(FluidApp::on_window_closed));
}

void FluidApp::setup_startup_graph(fw::scaffold_startup_node& parent)
{
    parent.add_child(fw::scaffold_startup_node([&]() -> void
        {
            m_imGui = std::make_unique<mygui::Context>(&get_window());
            initialise_simulation();
        }));
}

void FluidApp::setup_update_graph(fw::scaffold_update_node& parent)
{
    static f64 display_fps_time_ms = 0.0;
    parent.add_child(fw::scaffold_update_node([&]() -> void
        {
            get_window().process_events();

            update_simulation();

            m_imGui->begin_frame();
            update_simulation_debug();
            m_imGui->end_frame();

            render_simulation();
        }));
}

void FluidApp::setup_shutdown_graph(fw::scaffold_shutdown_node& parent)
{
    parent.add_child(fw::scaffold_shutdown_node([&]() -> void
        {
            shutdown_simulation();
            m_imGui = nullptr;
        }));
}

void FluidApp::initialise_simulation()
{
    FluidSimSettings2D settings{ };
    settings.dimensions = { u32_cast(m_simulationWidth), u32_cast(m_simulationHeight) };
    settings.node_capacity = m_simulationNodes;
    settings.additional_previous_iterations = 0;

    m_simulation = std::make_unique<FluidSim2D>(settings);

    gfx::driver::wait_idle();

    gfx::memory_info nodeBuffersMemInfo = gfx::memory_info::create_as_buffer(sizeof(FluidNode2D) * m_simulationNodes, gfx::format::R32G32B32A32_SFLOAT, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::BUFFER_USAGE_STORAGE);
    gfx::memory_info windowBuffersMemInfo = gfx::memory_info::create_as_buffer(sizeof(FrameInfo), gfx::format::R32G32_SFLOAT, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::BUFFER_USAGE_STORAGE);
    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
    {
        // Create and resize our node buffers.
        if( m_nodeBuffers[idx] )
        {
            m_nodeBuffers[idx]->unmap();
            gfx::buffer::destroy(m_nodeBuffers[idx]);
        }
        else
        {
            m_nodeBuffers[idx] = new gfx::buffer;
        }

        *m_nodeBuffers[idx] = gfx::buffer::create(nodeBuffersMemInfo);
        if( !m_nodeBuffers[idx]->is_mapped() )
            m_nodeBuffers[idx]->map();

        // Create our window info buffers if we haven't already.
        if( !m_frameInfoBuffers[idx] )
        {
            m_frameInfoBuffers[idx] = new gfx::buffer;
            *m_frameInfoBuffers[idx] = gfx::buffer::create(windowBuffersMemInfo);
            m_frameInfoBuffers[idx]->map();
        }
    }

    // This marks everything as dirty
    update_simulation_settings();

    if( m_visualiseProgram == nullptr )
    {
        // Need to initialise our program stuff.
        gfx::program_mgr::initialise("C:\\Users\\Jake\\Documents\\Projects\\UnnamedGame\\src\\game\\shaderdev\\compiled\\");
        gfx::program_mgr::load("fluid_viz_basic_2d.fxcp");
        m_visualiseProgram = const_cast<gfx::program*>(gfx::program_mgr::find_program(dt::hash_string32("fluid_viz_basic_2d")));

        m_descriptorPool.initialise(m_visualiseProgram->get_pass(0).get_descriptor_table(gfx::DESCRIPTOR_TABLE_PER_FRAME), GFX_RI_FRAMES_IN_FLIGHT);
        for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
        {
            m_programTable[idx] = m_descriptorPool.allocate();
            m_programTable[idx]->set_buffer(dt::hash_string32("in_nodeList"), m_nodeBuffers[idx]);
            m_programTable[idx]->set_buffer(dt::hash_string32("in_frameInfo"), m_frameInfoBuffers[idx]);

            m_programTable[idx]->write();
        }
    }

    // Insert our initial nodes
    for( u32 idx = 0; idx < m_simulationNodes; idx++ )
    {
        f32 height = (m_simulationHeight / 2.f);
        f32 width = (idx + 1) * (m_simulationWidth / f32_cast(m_simulationNodes + 1));
        FluidNode2D node
        {
            .position = { width, height },
            .velocity = { (f32_cast(rand()) / RAND_MAX) * 10 - 5, (f32_cast(rand()) / RAND_MAX) * 5 - 2.5 },
            .node_radius = m_simulationNodeRadius
        };
        
        m_simulation->InsertNode(node);
    }
}

void FluidApp::update_simulation()
{
    if( m_simulationPaused )
        return;

    FluidSimExternalForce2D gravity{ FluidSimExternalForceType2D::GravityForce };
    gravity.asGravityForce.acceleration = m_gravityValue;

    std::vector<FluidSimExternalForce2D> forces;
    forces.push_back(gravity);

    m_simulation->Simulate(fw::Time::delta_time(), forces);
}

void FluidApp::update_simulation_debug()
{
    ImGui::Begin("Stats");
    static f64 delta_time = fw::Time::delta_time();
    static f64 update_display_rolling = 1.0;
    update_display_rolling += fw::Time::delta_time();
    if( update_display_rolling >= 1.0 )
    {
        update_display_rolling = 0.0;
        delta_time = fw::Time::delta_time();
    }

    ImGui::LabelText("Delta Time", "%.2fs %.2fms %.2fus", delta_time, delta_time * 1e3, delta_time * 1e6);
    ImGui::LabelText("FPS", "%.2f", 1.0 / delta_time);
    ImGui::End();

    ImGui::Begin("Fluid Simulation Settings");
    ImGui::SliderInt("Node Count", (int*)&m_simulationNodes, 0, 500);
    ImGui::SliderFloat("Node Radius", &m_simulationNodeRadius, 0.f, 10.f);
    ImGui::SliderFloat2("Dimensions", &m_simulationWidth, 0.f, 100.f);
    ImGui::SliderFloat("Gravity", &m_gravityValue, 0.f, 20.f);
    ImGui::Checkbox("Paused?", &m_simulationPaused);

    if( ImGui::Button("Reset Simulation") )
        initialise_simulation();

    if( ImGui::Button("Update Simulation") )
        update_simulation_settings();

    ImGui::End();
}

void FluidApp::shutdown_simulation()
{
    m_simulation.reset();

    gfx::driver::wait_idle();
    GFX_CALL(destroy_descriptor_pool, &m_descriptorPool);
    m_visualiseProgram = nullptr;
    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
    {
        m_nodeBuffers[idx]->unmap();
        gfx::buffer::destroy(m_nodeBuffers[idx]);
        delete m_nodeBuffers[idx];
        m_nodeBuffers[idx] = nullptr;

        m_frameInfoBuffers[idx]->unmap();
        gfx::buffer::destroy(m_frameInfoBuffers[idx]);
        delete m_frameInfoBuffers[idx];
        m_frameInfoBuffers[idx] = nullptr;

        m_programTable[idx] = nullptr;
    }

    gfx::program_mgr::shutdown();
}

void FluidApp::render_simulation()
{
    gfx::fw::render_interface::begin_frame();
    update_simulation_buffers();

    u32 frame_idx = gfx::fw::render_interface::get_current_frame_index();

    // Do the rendering bit
    gfx::texture_view* texture_view = gfx::fw::render_interface::get_active_swapchain_texture_view();
    gfx::texture* texture = const_cast<gfx::texture*>(texture_view->get_resource());

    // make swapchain renderable
    RI_GraphicsContext.texture_layout_transition(texture, gfx::TEXTURE_LAYOUT_COLOR_ATTACHMENT);

    gfx::texture_attachment attachment
    {
        .view = texture_view,
        .load = gfx::LOAD_OP_CLEAR,
        .store = gfx::STORE_OP_STORE,
    };
    RI_GraphicsContext.begin_rendering({ attachment }, nullptr);

    RI_GraphicsContext.bind_program(m_visualiseProgram, 0);
    RI_GraphicsContext.bind_descriptor_table(&m_visualiseProgram->get_pass(0), m_programTable[frame_idx], gfx::DESCRIPTOR_TABLE_PER_FRAME);

    RI_GraphicsContext.set_viewport(0.f, 0.f, f32_cast(get_window().get_extent().x), f32_cast(get_window().get_extent().y), 0.f, 1.f);
    RI_GraphicsContext.set_scissor(0, 0, u32_cast(get_window().get_extent().x), u32_cast(get_window().get_extent().y));

    // We're drawing a square, 6 vertices per square.
    RI_GraphicsContext.draw(6, m_simulation->GetCurrentIterationData().GetNodeCount(), 0, 0);

    // Render ImGui above what we've just done.
    render_simulation_debug();

    RI_GraphicsContext.end_rendering();
    gfx::fw::render_interface::end_frame();
}

void FluidApp::render_simulation_debug()
{
    m_imGui->render(RI_GraphicsContext);
}

void FluidApp::update_simulation_settings()
{
    m_simulation->UpdateSettings({ u32_cast(m_simulationWidth), u32_cast(m_simulationHeight) });
    m_simulation->UpdateNodeRadius(m_simulationNodeRadius);

    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
        m_frameInfoDirty[idx] = true;
}

void FluidApp::update_simulation_buffers()
{
    u32 frame_idx = gfx::fw::render_interface::get_current_frame_index();

    // Write our window info.
    if( m_frameInfoDirty[frame_idx] )
    {
        m_frameInfoDirty[frame_idx] = false;

        gfx::buffer* infoBuffer = m_frameInfoBuffers[frame_idx];

        FrameInfo builtWindowInfo{ };
        builtWindowInfo.sim_to_local =
        {
            1.f / m_simulationWidth,
            1.f / m_simulationHeight,
        };

        memcpy(infoBuffer->get_mapped(), &builtWindowInfo, sizeof(FrameInfo));
    }

    // Write our node buffer
    gfx::buffer* node_buffer = m_nodeBuffers[frame_idx];
    memcpy(node_buffer->get_mapped(), m_simulation->GetCurrentIterationData().GetNodes(), m_simulation->GetCurrentIterationData().GetNodeCount() * sizeof(FluidNode2D));
}

bool FluidApp::on_window_resize(WindowResizeEvent& e)
{
    return false;
}

bool FluidApp::on_window_closed(WindowClosedEvent& e)
{
    set_should_close();
    return true;
}

fw::window::state FluidApp::get_window_startup_state()
{
    return
    {
        "Particle App",
        fw::window::mode::windowed,
        false,
        false,
        { 0, 0 },
        { 1200, 1200 },
        fw::cursor_lock_state::none,
        BIND_EVENT_FN(FluidApp::on_event),
    };
}