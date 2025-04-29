#include "CrawlerGame.h"

#include "imgui.h"
#include "input/imgui/imgui_bindings.h"
#include "input/Input.h"
#include "gfx_fw/render_interface.h"
#include "basic/Time.h"

bool CrawlerGame::on_game_startup()
{
	m_imGui = std::make_unique<mygui::Context>(&get_window());
	return true;
}

void CrawlerGame::update_impl(float dt)
{
	static f32 rolling = 0.f;
	rolling += dt;
	if( rolling >= 0.2f )
	{
		get_window().set_title(std::format("Crawler Game: {} fps", 1.f / dt));
		rolling = 0.f;
	}

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

		m_cursor.x += mouseX;
		m_cursor.y += mouseY;
	}

	m_position.x += move.x;
	m_position.y += move.y;
	m_position.z += move.z;
}

void CrawlerGame::on_event(Event& e)
{
	Input::register_event(e);
	mygui::dispatch_event(e);
}

void CrawlerGame::on_game_shutdown()
{
}

void CrawlerGame::setup_update_graph(fw::scaffold_update_node& parent)
{
	parent.add_child(fw::scaffold_update_node([&]() -> void
		{
			get_window().process_events();

			update(fw::Time::delta_time());
		}));
}

bool CrawlerGame::update(f64 deltaTime)
{
	update_impl(f32_cast(deltaTime));
	Input::tick();

	m_imGui->begin_frame();

	ImGui::Begin("Values");
	ImGui::DragFloat3("Position", &m_position.x);
	ImGui::DragFloat2("Mouse", &m_cursor.x);
	ImGui::End();

	m_imGui->end_frame();

	gfx::fw::render_interface::begin_frame();

	gfx::texture_view* swap_view = gfx::fw::render_interface::get_active_swapchain_texture_view();
	gfx::texture* swap_tex = const_cast<gfx::texture*>(swap_view->get_resource());
	gfx::texture_attachment attachment
	{
		.view = swap_view,
		.load = gfx::LOAD_OP_CLEAR,
		.store = gfx::STORE_OP_STORE,
	};

	// swapchain -> renderable
	RI_GraphicsContext.texture_layout_transition(swap_tex, gfx::TEXTURE_LAYOUT_COLOR_ATTACHMENT);
	RI_GraphicsContext.begin_rendering({ attachment }, nullptr);

	m_imGui->render(RI_GraphicsContext);

	RI_GraphicsContext.end_rendering();
	gfx::fw::render_interface::end_frame();
	return true;
}

bool CrawlerGame::on_window_resize(WindowResizeEvent& e)
{
	return false;
}

fw::window::state CrawlerGame::get_window_startup_state()
{
	return
	{
		"Testing Title",
		fw::window::mode::windowed,
		true,
		false,
		{ 0, 0 },
		{ 1600, 1200 },
		fw::cursor_lock_state::none,
		std::bind(&CrawlerGame::on_event, this, std::placeholders::_1),
	};
}