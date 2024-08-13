#include "CrawlerGame.h"

#include "input/Input.h"
#include "scene/spatial/Entity.h"
#include "scene/spatial/components/RenderableMesh.h"

static Window::Properties default_window_properties
{
	"Crawler Game",
	Window::Mode::Windowed,
	true,
	Window::VSync::Default,
	Window::Position{ 0, 0 },
	Window::Extent{ 1600, 1200 }
};

CrawlerGame::CrawlerGame() :
	WindowedApplication("Crawler Game", default_window_properties),
	m_beginFrame()
{ }

CrawlerGame::~CrawlerGame()
{ }

void CrawlerGame::on_app_startup()
{
	// setup game logic/scene
	// 
	// if(!headless)
	//   create render context obj
	//   create whatever renderer
	//   create imgui context

	m_beginFrame = sys::now();

	debug_setup();
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

	glm::quat qRotation(glm::vec3(0.f));
	qRotation *= glm::angleAxis(m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
	qRotation *= glm::angleAxis(m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
	qRotation *= glm::angleAxis(m_rotation.z, glm::vec3(0.f, 0.f, 1.f));

	glm::vec3 camFwd = glm::vec3(0.f, 0.f, 1.f) * qRotation;
	glm::vec3 camRght = glm::vec3(1.f, 0.f, 0.f) * qRotation;

	if( Input::get_mouse_button_pressed(1) )
	{
		Input::set_cursor_lock_state(get_window(), CursorLockState::LOCKED);
	}
	else if( Input::get_mouse_button_released(1) )
	{
		Input::set_cursor_lock_state(get_window(), CursorLockState::NONE);
	}

	if( Input::get_mouse_button_down(1) )
	{
		float sens = .1f;

		float mouseX = static_cast<float>(sens * Input::get_mouse_move_horizontal());
		float mouseY = static_cast<float>(sens * Input::get_mouse_move_vertical());

		m_rotation.y += glm::radians(mouseX);
		m_rotation.x += glm::radians(mouseY);
	}

	m_position += (camRght * move.x) + (camFwd * move.z) + (glm::vec3(0.f, 1.f, 0.f) * move.y);
}

void CrawlerGame::on_event(Event& e)
{
	Input::register_event(e);
}

void CrawlerGame::on_app_shutdown()
{ }

void CrawlerGame::update()
{
	// Calculate delta time
	sys::moment frameStart = sys::now();
	float deltatime = std::chrono::duration_cast<std::chrono::nanoseconds>(frameStart - m_beginFrame).count() / 1e9f;
	m_beginFrame = frameStart;

	get_window().process_events();

	m_scene->pre_update();

	update_impl(deltatime);
	Input::tick();

	m_renderer->pre_render(m_position, m_rotation);
	m_renderer->render();
}

bool CrawlerGame::on_window_resize(WindowResizeEvent& e)
{
	return false;
}


#include "loaders/obj_waveform.h"
void CrawlerGame::debug_setup()
{
	m_scene = std::make_unique<fw::Scene>();

	// fw::BlueprintManager::initialise();

	{
		// vk::RenderContext
		std::vector<VkPresentModeKHR> presentModes =
		{ VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };

		std::vector<VkSurfaceFormatKHR> surfaceFormats =
		{ { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } };

		m_context = std::make_unique<vk::RenderContext>(
			get_device(),
			get_window().create_surface(get_instance()),
			get_window(),
			presentModes,
			surfaceFormats,
			vk::RenderTarget::default_create_function);
	}

	m_scene = std::make_unique<fw::Scene>();
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

	fw::EntityDef entdef1
	{
		glm::vec3(0.f),
		glm::vec3(1.f),
		glm::vec3(0.f)
	};

	fw::Entity* entity = m_scene->add_entity(entdef1);
	entity->add_component<fw::RenderableMeshComponent>("assets/models/heli.obj");

	m_renderer->add_scene(m_scene.get());
}