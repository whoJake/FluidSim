#include "CrawlerGame.h"

#include "input/Input.h"
#include "scene/spatial/Entity.h"

#include "scene/spatial/components/RenderableMesh.h"
#include "scene/spatial/components/Camera.h"

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
}

void CrawlerGame::on_event(Event& e)
{
	Input::register_event(e);
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

	// we need this to block on the new frame, otherwise we can overwrite data during pre_render
	get_graphics_handles().context->begin_frame();

	m_renderer->pre_render(m_scene.get(), f32_cast(deltaTime));
	m_renderer->render();

	glm::vec3 pos = m_camera->transform().get_position();
	// SYSLOG_INFO("Camera location x:{} y:{} z:{}", pos.x, pos.y, pos.z);
	return true;
}

bool CrawlerGame::on_window_resize(WindowResizeEvent& e)
{
	return false;
}


#include "loaders/obj_waveform.h"
void CrawlerGame::debug_setup()
{
	/*
	m_renderer = std::make_unique<fw::SceneRenderer>(*m_context);

	fw::gfx::ShaderDefinition shader
	{
		mtl::hash_string("shader1"),
		"assets/shaders/modules/vertex/basic_unknown.vert",
		"assets/shaders/modules/fragment/basic_unknown.frag",
		""
	};

	m_renderer->initialise_shaders({ shader });

	fw::gfx::MaterialDefinition material
	{
		mtl::hash_string("material1"),
		mtl::hash_string("shader1"),
		0
	};

	m_renderer->register_material(material);
	*/

	m_scene = std::make_unique<Scene>();

	EntityDef entdef1
	{
		glm::vec3(10.f, -5.f, 1.f),
		glm::vec3(0.2f),
		glm::vec3(0.f)
	};

	EntityDef entdef2
	{
		glm::vec3(-10.f, 0.f, -15.f),
		glm::vec3(0.8f),
		glm::vec3(0.f, glm::radians(180.f), 0.f)
	};

	EntityDef entdef3
	{
		glm::vec3(-40.f, -10.f, -10.f),
		glm::vec3(0.3f),
		glm::vec3(glm::radians(270.f), glm::radians(45.f), 0.f)

	};

	EntityDef entdef4
	{
		glm::vec3(0.f, 0.f, 5.f),
		glm::vec3(1.f),
		glm::vec3(0.f)
	};

	Entity* entity1 = m_scene->add_entity(entdef1);
	RenderableMeshComponent* mesh1 = entity1->add_component<RenderableMeshComponent>("assets/models/heli.obj");
	mesh1->load();

	Entity* entity2 = m_scene->add_entity(entdef2);
	RenderableMeshComponent* mesh2 = entity2->add_component<RenderableMeshComponent>("assets/models/apple.obj");
	mesh2->load();

	Entity* entity3 = m_scene->add_entity(entdef3);
	RenderableMeshComponent* mesh3 = entity3->add_component<RenderableMeshComponent>("assets/models/skull.obj");
	mesh3->load();

	m_camera = m_scene->add_entity(entdef4);
	m_camera->add_component<CameraComponent>(90.f, 1600.f/1200.f);

	m_renderer = std::make_unique<Renderer>(*get_graphics_handles().context);
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