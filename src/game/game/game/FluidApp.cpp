#include "FluidApp.h"

#include "input/imgui/imgui_bindings.h"
#include "input/Input.h"
#include "basic/Time.h"

#include "gfx_core/driver.h"
#include "gfx_fw/program_mgr.h"

void FluidApp::on_event(Event& e)
{
    Input::tick();

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
            initialise_app();

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

            m_imGui->begin_frame();
            update_simulation_debug();
            m_imGui->end_frame();

            update_simulation();

            update_movement();
            render_simulation();
        }));
}

void FluidApp::setup_shutdown_graph(fw::scaffold_shutdown_node& parent)
{
    parent.add_child(fw::scaffold_shutdown_node([&]() -> void
        {
            shutdown_simulation();
            m_imGui = nullptr;

            shutdown_app();
        }));
}

void FluidApp::initialise_app()
{
    // Need to initialise our program stuff.
    gfx::program_mgr::initialise("C:\\Users\\Jake\\Documents\\Projects\\UnnamedGame\\src\\game\\shaderdev\\compiled\\");
    gfx::program_mgr::load("fluid_viz_basic_2d.fxcp");
    m_visualiseProgram = const_cast<gfx::program*>(gfx::program_mgr::find_program(dt::hash_string32("fluid_viz_basic_2d")));

    m_descriptorPool.initialise(m_visualiseProgram->get_pass(0).get_descriptor_table(gfx::DESCRIPTOR_TABLE_PER_FRAME), GFX_RI_FRAMES_IN_FLIGHT);
    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
    {
        m_programTable[idx] = m_descriptorPool.allocate();
    }

}

void FluidApp::shutdown_app()
{
    gfx::driver::wait_idle();
    GFX_CALL(destroy_descriptor_pool, &m_descriptorPool);
    gfx::program_mgr::shutdown();
}

void FluidApp::initialise_simulation()
{
    FluidSimSettings2D settings{ };
    settings.dimensions = { u32_cast(m_simulationWidth), u32_cast(m_simulationHeight) };
    settings.node_capacity = m_simulationNodes;
    settings.additional_previous_iterations = 0;

    m_simulation = std::make_unique<FluidSim2D>(settings);
    m_viewport = Viewport2D({ 1200, 1200 }, { 0, 0 }, { 100, 100 });


    gfx::driver::wait_idle();

    gfx::memory_info viewportBufMemInfo = gfx::memory_info::create_as_buffer(sizeof(Viewport2D), gfx::format::R32G32B32A32_SFLOAT, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::BUFFER_USAGE_STORAGE);
    gfx::memory_info nodeBuffersMemInfo = gfx::memory_info::create_as_buffer(sizeof(FluidNode2D) * m_simulationNodes, gfx::format::R32G32B32A32_SFLOAT, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::BUFFER_USAGE_STORAGE);
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
        if( !m_viewportBuffers[idx] )
        {
            m_viewportBuffers[idx] = new gfx::buffer;
            *m_viewportBuffers[idx] = gfx::buffer::create(viewportBufMemInfo);
            m_viewportBuffers[idx]->map();
        }

        m_programTable[idx]->set_buffer(dt::hash_string32("g_viewport"), m_viewportBuffers[idx]);
        m_programTable[idx]->set_buffer(dt::hash_string32("in_nodeList"), m_nodeBuffers[idx]);

        m_programTable[idx]->write();
    }

    // This marks everything as dirty
    update_simulation_settings();

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

    ImGui::Begin("Controls");
    ImGui::InputFloat("Move Sensitivity", &m_moveSensitivity);
    ImGui::InputFloat("Zoom Sensitivity", &m_zoomSensitivity);
    ImGui::End();

    ImGui::Begin("Fluid Simulation Settings");
    ImGui::SliderInt("Node Count", (int*)&m_simulationNodes, 0, 500);
    ImGui::SliderFloat("Node Radius", &m_simulationNodeRadius, 0.f, 10.f);
    ImGui::SliderFloat2("Dimensions", &m_simulationWidth, 0.f, 100.f);
    ImGui::SliderFloat("Gravity", &m_gravityValue, 0.f, 20.f);
    ImGui::Checkbox("Paused?", &m_simulationPaused);

    if( ImGui::Button("Reset Simulation") )
    {
        m_simulationPaused = true;
        shutdown_simulation();
        initialise_simulation();
    }

    if( ImGui::Button("Update Simulation") )
        update_simulation_settings();

    ImGui::End();
}

void FluidApp::shutdown_simulation()
{
    m_simulation.reset();

    gfx::driver::wait_idle();
    for( u32 idx = 0; idx < GFX_RI_FRAMES_IN_FLIGHT; idx++ )
    {
        m_nodeBuffers[idx]->unmap();
        gfx::buffer::destroy(m_nodeBuffers[idx]);
        delete m_nodeBuffers[idx];
        m_nodeBuffers[idx] = nullptr;

        m_viewportBuffers[idx]->unmap();
        gfx::buffer::destroy(m_viewportBuffers[idx]);
        delete m_viewportBuffers[idx];
        m_viewportBuffers[idx] = nullptr;
    }
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
}

void FluidApp::update_simulation_buffers()
{
    u32 frame_idx = gfx::fw::render_interface::get_current_frame_index();

    m_viewport.update_matrices();
    gfx::buffer* viewport_buffer = m_viewportBuffers[frame_idx];
    memcpy(viewport_buffer->get_mapped(), &m_viewport, sizeof(Viewport2D));

    // Write our node buffer
    gfx::buffer* node_buffer = m_nodeBuffers[frame_idx];
    memcpy(node_buffer->get_mapped(), m_simulation->GetCurrentIterationData().GetNodes(), m_simulation->GetCurrentIterationData().GetNodeCount() * sizeof(FluidNode2D));
}

void FluidApp::update_movement()
{
    if( Input::get_key_down(KeyCode::Up) )
    {
        glm::vec2 extent_before = m_viewport.get_view_extent();
        m_viewport.set_view_extent(extent_before - (glm::vec2(m_zoomSensitivity) * f32_cast(fw::Time::delta_time())));
        glm::vec2 extent_after = m_viewport.get_view_extent();

        glm::vec2 difference = extent_before - extent_after;
        m_viewport.set_view_position(m_viewport.get_view_position() - (difference / 2.f));
    }

    if( Input::get_key_down(KeyCode::Down) )
    {
        glm::vec2 extent_before = m_viewport.get_view_extent();
        m_viewport.set_view_extent(extent_before + (glm::vec2(m_zoomSensitivity) * f32_cast(fw::Time::delta_time())));

        glm::vec2 extent_after = m_viewport.get_view_extent();

        glm::vec2 difference = extent_after - extent_before;
        m_viewport.set_view_position(m_viewport.get_view_position() + (difference / 2.f));
    }

    f64 zoom = Input::get_mouse_scroll_vertical();
    glm::vec2 scroll{ 250.f, 250.f };
    scroll *= f32_cast(fw::Time::delta_time());
    scroll *= zoom;
    m_viewport.set_view_extent(m_viewport.get_view_extent() + scroll);

    f32 x_scale = m_viewport.get_view_extent().x / m_viewport.get_screen_extent().x;
    f32 y_scale = m_viewport.get_view_extent().y / m_viewport.get_screen_extent().y;

    glm::vec2 movement = glm::vec2(x_scale, y_scale) * m_moveSensitivity * f32_cast(fw::Time::delta_time());
    if( Input::get_key_down(KeyCode::A) && Input::get_key_down(KeyCode::D) )
    { }
    else if( Input::get_key_down(KeyCode::D) )
    {
        movement.x *= -1;
    }
    else if( Input::get_key_down(KeyCode::A) )
    { }
    else
    {
        movement.x *= 0;
    }

    if( Input::get_key_down(KeyCode::W) && Input::get_key_down(KeyCode::S) )
    { }
    else if( Input::get_key_down(KeyCode::W) )
    {
        movement.y *= -1;
    }
    else if( Input::get_key_down(KeyCode::S) )
    { }
    else
    {
        movement.y *= 0;
    }

    m_viewport.set_view_position(m_viewport.get_view_position() + movement);
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