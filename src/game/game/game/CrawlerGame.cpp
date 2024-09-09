#include "CrawlerGame.h"

#include "input/Input.h"
#include "scene/spatial/Entity.h"

#include "scene/spatial/components/RenderableMesh.h"
#include "scene/spatial/components/Camera.h"

#include "imgui.h"
#include "input/imgui/imgui_bindings.h"

bool CrawlerGame::on_game_startup()
{
	// setup game logic/scene
	// 
	// if(!headless)
	//   create render context obj
	//   create whatever renderer
	//   create imgui context

	debug_setup();
	return true;
}

void CrawlerGame::update_impl(float dt)
{
	get_window().set_title(std::format("Crawler Game: {} fps", 1.f / dt));

	glm::vec3 move{ };
	if( Input::get_key_down(KeyCode::A) )
		move.x -= 1;
	if( Input::get_key_down(KeyCode::D) )
		move.x += 1;
	if( Input::get_key_down(KeyCode::S) )
		move.z += 1;
	if( Input::get_key_down(KeyCode::W) )
		move.z -= 1;
	if( Input::get_key_down(KeyCode::Space) )
		move.y += 1;
	if( Input::get_key_down(KeyCode::LeftShift) )
		move.y -= 1;

	float speed = 10.f;
	move *= speed * dt;

	glm::vec3 camFwd = glm::vec3(0.f, 0.f, 1.f) * m_camera->transform().get_quat_rotation();
	glm::vec3 camRght = glm::vec3(1.f, 0.f, 0.f) * m_camera->transform().get_quat_rotation();

	if( Input::get_mouse_button_pressed(1) )
	{
		Input::set_cursor_lock_state(get_window(), fw::cursor_lock_state::locked);
	}
	else if( Input::get_mouse_button_released(1) )
	{
		Input::set_cursor_lock_state(get_window(), fw::cursor_lock_state::none);
	}

	if( Input::get_mouse_button_down(1) )
	{
		float sens = .1f;

		float mouseX = static_cast<float>(sens * Input::get_mouse_move_horizontal());
		float mouseY = static_cast<float>(sens * Input::get_mouse_move_vertical());

		m_camera->transform().rotate({ 0.f, glm::radians(mouseX), 0.f });
		m_camera->transform().rotate(glm::radians(mouseY) * camRght);
	}

	m_camera->transform().move((camRght * move.x) + (camFwd * move.z) + (glm::vec3(0.f, 1.f, 0.f) * move.y));

	f32 rotSpeed = 90.f;
	m_scene->for_each([=](Entity* entity)
		{
			if( entity->has_component<CameraComponent>() )
				return;
			entity->transform().rotate(glm::vec3(0.f, glm::radians(dt * rotSpeed), 0.f));
		});
}

void CrawlerGame::on_event(Event& e)
{
	Input::register_event(e);
	mygui::dispatch_event(e);
}

void CrawlerGame::on_game_shutdown()
{
	m_scene.reset();
	m_renderer.reset();
}

bool CrawlerGame::update(f64 deltaTime)
{
	// Calculate delta time
	get_window().process_events();

	m_scene->pre_update();

	update_impl(f32_cast(deltaTime));
	Input::tick();

	glm::vec3 pos = m_camera->transform().get_position();
	ImGui::Begin("Camera");
	ImGui::DragFloat3("Position", &pos.x);
	ImGui::End();

	m_scene->draw_debug_panel();

	// we need this to block on the new frame, otherwise we can overwrite data during pre_render
	get_graphics_handles().context->begin_frame();

	m_renderer->pre_render(m_scene.get(), f32_cast(deltaTime));
	m_renderer->render();

	return true;
}

bool CrawlerGame::on_window_resize(WindowResizeEvent& e)
{
	return false;
}


#include "loaders/obj_waveform.h"
void CrawlerGame::debug_setup()
{
	m_scene = std::make_unique<Scene>();

	f32 min = -25.f;
	f32 max = 25.f;
	i32 inc = 25;
	f32 step = (max - min) / inc;
	for( i32 idx = 0; idx < inc; idx++ )
	{
		f32 x = min + (step * idx);
		for( i32 idy = 0; idy < inc; idy++ )
		{
			f32 y = min + (step * idy);
			f32 rot = (rand() / f32_cast(RAND_MAX)) * 180.f;
			EntityDef def
			{
				glm::vec3(x, 0.f, y),
				glm::vec3(1.f),
				glm::vec3(0.f, glm::radians(rot), 0.f)
			};
			Entity* entity = m_scene->add_entity(def);
			entity->add_component<RenderableMeshComponent>("assets/models/cube.obj");
		}
	}

	EntityDef camera
	{
		glm::vec3(0.f, 0.f, 5.f),
		glm::vec3(1.f),
		glm::vec3(0.f)
	};

	m_camera = m_scene->add_entity(camera);
	m_camera->add_component<CameraComponent>(90.f, 1600.f/1200.f);

	m_renderer = std::make_unique<Renderer>(*get_graphics_handles().context);
#if CFG_ENABLE_IMGUI
	m_renderer->init_debug_imgui(&get_window());
#endif
}

fw::game::options CrawlerGame::get_startup_options()
{
	VkPhysicalDeviceFeatures features{ };
	features.fillModeNonSolid = true;

	return
	{
		"Crawler Game",
		{ },
		{ },
		{ },
		features,
	};
}

fw::window::state CrawlerGame::get_window_startup_state()
{
	return
	{
		"Crawler Game Title",
		fw::window::mode::windowed,
		true,
		false,
		{ 0, 0 },
		{ 1600, 1200 },
		fw::cursor_lock_state::none,
		std::bind(&CrawlerGame::on_event, this, std::placeholders::_1),
	};
}