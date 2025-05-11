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

            Input::tick();
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
    FluidSimOptions2D options{ };
    options.extent = glm::f32vec2(m_simWidth, m_simHeight);
    options.grid_extent = glm::f32vec2(m_smoothingRadius, m_smoothingRadius);
    options.should_bounce = m_boundryBounce;
    options.dampening_factor = m_dampeningFactor;
    options.smoothing_radius = m_smoothingRadius;
    options.target_density = m_targetDensity;
    options.pressure_multiplier = m_pressureMultiplier;

    m_simulation = std::make_unique<FluidSim2D>(options);
    m_viewport = Viewport2D({ 1200, 1200 }, { 0, 0 }, { m_simWidth, m_simHeight });


    gfx::driver::wait_idle();

    gfx::memory_info viewportBufMemInfo = gfx::memory_info::create_as_buffer(sizeof(Viewport2D), gfx::format::R32G32B32A32_SFLOAT, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::BUFFER_USAGE_STORAGE);
    gfx::memory_info positionsBufMemInfo = gfx::memory_info::create_as_buffer(sizeof(glm::f32vec4) * m_nodeCount, gfx::format::R32G32B32A32_SFLOAT, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::BUFFER_USAGE_STORAGE);
    gfx::memory_info nodeBuffersMemInfo = gfx::memory_info::create_as_buffer(sizeof(FluidNodeInfo2D) * m_nodeCount, gfx::format::R32G32B32A32_SFLOAT, gfx::MEMORY_TYPE_CPU_VISIBLE, gfx::BUFFER_USAGE_STORAGE);
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

        if( m_positionsBuffers[idx] )
        {
            m_positionsBuffers[idx]->unmap();
            gfx::buffer::destroy(m_positionsBuffers[idx]);
        }
        else
        {
            m_positionsBuffers[idx] = new gfx::buffer;
        }

        *m_positionsBuffers[idx] = gfx::buffer::create(positionsBufMemInfo);
        if( !m_positionsBuffers[idx]->is_mapped() )
            m_positionsBuffers[idx]->map();

        // Create our window info buffers if we haven't already.
        if( !m_viewportBuffers[idx] )
        {
            m_viewportBuffers[idx] = new gfx::buffer;
            *m_viewportBuffers[idx] = gfx::buffer::create(viewportBufMemInfo);
            m_viewportBuffers[idx]->map();
        }

        m_programTable[idx]->set_buffer(dt::hash_string32("g_viewport"), m_viewportBuffers[idx]);
        m_programTable[idx]->set_buffer(dt::hash_string32("in_positions"), m_positionsBuffers[idx]);
        m_programTable[idx]->set_buffer(dt::hash_string32("in_nodeList"), m_nodeBuffers[idx]);

        m_programTable[idx]->write();
    }

    distribute_nodes();
}

void FluidApp::update_simulation()
{
    if( !m_simPaused )
    {
        FluidSimExternalForce2D gravity{ FluidSimExternalForceType2D::GravityForce };
        gravity.asGravityForce.acceleration = m_gravityValue;

        std::vector<FluidSimExternalForce2D> forces;
        forces.push_back(gravity);

        m_simulation->Simulate(fw::Time::delta_time(), forces);
    }

    // Debug affects
    std::vector<FluidSimExternalDebug2D> debugs;
    if( m_visualiseType == VisualiseType::FlatColor )
    {
        FluidSimExternalDebug2D set_color{ FluidSimExternalDebugType2D::SetColor };
        set_color.asSetColor.color = m_nodeColor;
        debugs.push_back(set_color);
    }
    else if( m_visualiseType == VisualiseType::MouseSelection )
    {
        FluidSimExternalDebug2D set_color{ FluidSimExternalDebugType2D::SetColor };
        set_color.asSetColor.color = m_nodeColor;
        debugs.push_back(set_color);

        FluidSimExternalDebug2D paint_cursor{ FluidSimExternalDebugType2D::PointPaint };
        paint_cursor.asPointPaint.color = m_paintColor;
        paint_cursor.asPointPaint.position = m_mouseWorldPosition;
        paint_cursor.asPointPaint.radius = m_paintRadius;
        debugs.push_back(paint_cursor);
    }
    else if( m_visualiseType == VisualiseType::DensityView )
    {
        FluidSimExternalDebug2D density_view{ FluidSimExternalDebugType2D::SetDensityColor };
        density_view.asDensityColor.min_density = m_minDensityDisplay;
        density_view.asDensityColor.max_density = m_maxDensityDisplay;
        density_view.asDensityColor.min_color = m_densityMinColor;
        density_view.asDensityColor.max_color = m_densityMaxColor;
        debugs.push_back(density_view);
    }
    else
    {
        FluidSimExternalDebug2D set_color{ FluidSimExternalDebugType2D::SetColor };
        set_color.asSetColor.color = { 1.f, 1.f, 1.f };
        debugs.push_back(set_color);
    }

    m_simulation->ApplyDebug(debugs);
}

void FluidApp::update_simulation_debug()
{
    static bool show_controls = false;
    static bool show_dist_debug = false;
    static bool show_visual = false;

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

    ImGui::Begin("Options");

    ImGui::SliderFloat("Gravity", &m_gravityValue, 0.f, 20.f);
    ImGui::Checkbox("Paused?", &m_simPaused);

    if( ImGui::Button("Reset Simulation") )
    {
        m_simPaused = true;
        shutdown_simulation();
        initialise_simulation();
    }

    ImGui::Checkbox("Show Node Setup", &show_dist_debug);
    ImGui::Checkbox("Display Controls", &show_controls);
    ImGui::Checkbox("Display Visualisers", &show_visual);

    ImGui::End();

    if( show_visual )
    {
        ImGui::Begin("Visualisers");
        const char* labels[3] =
        {
            "Flat Color",
            "Mouse Selection",
            "Density View",
        };

        ImGui::Combo("Types", (int*)&m_visualiseType, labels, 3);

        if( m_visualiseType == VisualiseType::FlatColor )
        {
            ImGui::ColorEdit3("Color", &m_nodeColor.x);
        }
        else if( m_visualiseType == VisualiseType::MouseSelection )
        {
            ImGui::SliderFloat("Selection Radius", &m_paintRadius, 0.f, std::max(m_simWidth, m_simHeight));
            ImGui::ColorEdit3("Color", &m_nodeColor.x);
            ImGui::ColorEdit3("Selected Color", &m_paintColor.x);
        }
        else if( m_visualiseType == VisualiseType::DensityView )
        {
            ImGui::SliderFloat2("Min/Max Density", &m_minDensityDisplay, 0.f, 10.f);
            ImGui::ColorEdit3("Min Color", &m_densityMinColor.x);
            ImGui::ColorEdit3("Max Color", &m_densityMaxColor.x);
        }
        ImGui::End();
    }

    if( show_dist_debug )
    {
        ImGui::Begin("Node Startup Config");
        ImGui::SliderFloat("Node Radius", &m_nodeRadius, 0.f, 10.f);
        ImGui::SliderFloat("Smoothing Radius", &m_smoothingRadius, m_nodeRadius, std::min(m_simWidth, m_simHeight));
        ImGui::SliderFloat2("Dimensions", &m_simWidth, 0.f, 100.f);
        ImGui::SliderFloat("Target Density", &m_targetDensity, 0.f, 10.f);
        ImGui::SliderFloat("Pressure Multiplier", &m_pressureMultiplier, 0.f, 50.f);

        ImGui::Checkbox("Should bounce?", &m_boundryBounce);
        if( m_boundryBounce )
            ImGui::SliderFloat("Damping Factor", &m_dampeningFactor, 0.f, 1.f);

        distribute_nodes_debug();
        ImGui::End();
    }

    if( show_controls )
    {
        ImGui::Begin("Controls");
        ImGui::InputFloat("Move Sensitivity", &m_moveSensitivity);
        ImGui::InputFloat("Zoom Sensitivity", &m_zoomSensitivity);
        ImGui::End();
    }
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

        m_positionsBuffers[idx]->unmap();
        gfx::buffer::destroy(m_positionsBuffers[idx]);
        delete m_positionsBuffers[idx];
        m_positionsBuffers[idx] = nullptr;

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
    RI_GraphicsContext.draw(6, m_simulation->GetNodeCount(), 0, 0);

    // Render ImGui above what we've just done.
    render_simulation_debug();

    RI_GraphicsContext.end_rendering();
    gfx::fw::render_interface::end_frame();
}

void FluidApp::render_simulation_debug()
{
    m_imGui->render(RI_GraphicsContext);
}

void FluidApp::update_simulation_buffers()
{
    u32 frame_idx = gfx::fw::render_interface::get_current_frame_index();

    m_viewport.update_matrices();
    gfx::buffer* viewport_buffer = m_viewportBuffers[frame_idx];
    memcpy(viewport_buffer->get_mapped(), &m_viewport, sizeof(Viewport2D));

    gfx::buffer* positions_buffer = m_positionsBuffers[frame_idx];
    memcpy(positions_buffer->get_mapped(), m_simulation->GetNodePositions().data(), m_simulation->GetNodeCount() * sizeof(glm::f32vec4));

    // Write our node buffer
    gfx::buffer* node_buffer = m_nodeBuffers[frame_idx];
    memcpy(node_buffer->get_mapped(), m_simulation->GetNodeInfos().data(), m_simulation->GetNodeCount() * sizeof(FluidNodeInfo2D));
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

    // Debugging, pass mouse position into the simulation
    f32 mouseX = f32_cast(Input::get_mouse_x());
    f32 mouseY = get_window().get_extent().y - f32_cast(Input::get_mouse_y());

    glm::f32vec2 mouse{ mouseX, mouseY };
    glm::f32vec2 local_mouse = mouse / m_viewport.get_screen_extent();

    m_mouseWorldPosition = -m_viewport.get_view_position() + (local_mouse * m_viewport.get_view_extent());

    if( Input::get_key_pressed(KeyCode::Space) )
        m_simPaused = !m_simPaused;
}

void FluidApp::distribute_nodes()
{
    switch( m_distributeTechnique )
    {
    case DistributionTechnique::Grid:
        distribute_nodes_grid();
        break;
    case DistributionTechnique::Circular:
        distribute_nodes_circular();
        break;
    case DistributionTechnique::Point:
        distribute_nodes_point();
        break;
    case DistributionTechnique::Random:
        distribute_nodes_random();
        break;
    }

    m_simulation->FinishInserting();
}

void FluidApp::distribute_nodes_debug()
{
    const char* labels[4] =
    {
        "Grid",
        "Circular",
        "Point",
        "Random"
    };

    ImGui::Combo("Technique", (int*)&m_distributeTechnique, labels, 4);
    ImGui::DragInt("Node Count", (int*)&m_nodeCount);

    switch( m_distributeTechnique )
    {
    case DistributionTechnique::Grid:
        distribute_nodes_grid_debug();
        break;
    case DistributionTechnique::Circular:
        distribute_nodes_circular_debug();
        break;
    case DistributionTechnique::Point:
        distribute_nodes_point_debug();
        break;
    }
}

void FluidApp::distribute_nodes_grid()
{
    glm::vec2 centre{ m_simWidth / 2.f, m_simHeight / 2.f };
    u32 side_length = u32_cast(std::ceil(std::sqrt(m_nodeCount)));

    glm::vec2 offset
    {
        side_length * m_dngSpacing / 2.f,
        side_length * m_dngSpacing / 2.f
    };

    for( u32 y = 0; y < side_length; y++ )
    {
        for( u32 x = 0; x < side_length; x++ )
        {
            if( (y * side_length) + x >= m_nodeCount )
                break;

            glm::vec2 local_position
            {
                x * m_dngSpacing,
                y * m_dngSpacing
            };

            glm::f32vec2 position = centre - offset + local_position;
            FluidNodeInfo2D node
            {
                .velocity = { 0.f, 0.f },
                .node_radius = m_nodeRadius,
                .density = 0.f,
                .mass = 1.f,
                .color = m_nodeColor
            };
            m_simulation->InsertNode(node, position);
        }
    }
}

void FluidApp::distribute_nodes_circular()
{
    glm::vec2 centre{ m_simWidth / 2.f, m_simHeight / 2.f };

    for( u32 idx = 0; idx < m_nodeCount; idx++ )
    {
        f32 rand_ang = (f32_cast(rand()) / RAND_MAX) * 3.14159f * 2;
        glm::vec2 rand_vec{ f32_cast(cos(rand_ang)), f32_cast(sin(rand_ang)) };

        glm::f32vec2 position = centre + (rand_vec * m_dncRadius);
        FluidNodeInfo2D node
        {
            .velocity = rand_vec * m_dncVelocityScale,
            .node_radius = m_nodeRadius,
            .density = 0.f,
            .mass = 1.f,
            .color = m_nodeColor
        };

        m_simulation->InsertNode(node, position);
    }
}

void FluidApp::distribute_nodes_point()
{
    glm::vec2 centre{ m_simWidth / 2.f, m_simHeight / 2.f };

    for( u32 idx = 0; idx < m_nodeCount; idx++ )
    {
        f32 rand_ang = (f32_cast(rand()) / RAND_MAX) * 3.14159f * 2;
        glm::vec2 rand_vec{ f32_cast(cos(rand_ang)), f32_cast(sin(rand_ang)) };

        glm::f32vec2 position = centre;
        FluidNodeInfo2D node
        {
            .velocity = rand_vec * m_dnpVelocityScale,
            .node_radius = m_nodeRadius,
            .density = 0.f,
            .mass = 1.f,
            .color = m_nodeColor
        };

        m_simulation->InsertNode(node, position);
    }
}

void FluidApp::distribute_nodes_random()
{
    for( u32 idx = 0; idx < m_nodeCount; idx++ )
    {
        f32 rand0 = f32_cast(rand()) / RAND_MAX;
        f32 rand1 = f32_cast(rand()) / RAND_MAX;

        glm::f32vec2 position
        {
            m_simWidth * rand0,
            m_simHeight * rand1
        };
        FluidNodeInfo2D node
        {
            .velocity = { 0.f, 0.f },
            .node_radius = m_nodeRadius,
            .density = 0.f,
            .mass = 1.f,
            .color = m_nodeColor
        };

        m_simulation->InsertNode(node, position);
    }
}

void FluidApp::distribute_nodes_grid_debug()
{
    ImGui::DragFloat("Spacing", &m_dngSpacing, 0.05f, m_nodeRadius * 2.f);
}

void FluidApp::distribute_nodes_circular_debug()
{
    ImGui::DragFloat("Velocity Scale", &m_dncVelocityScale);
    ImGui::DragFloat("Radius", &m_dncRadius);
}

void FluidApp::distribute_nodes_point_debug()
{
    ImGui::DragFloat("Velocity Scale", &m_dnpVelocityScale);
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