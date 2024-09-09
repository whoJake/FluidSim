#pragma once

#include "base/game.h"

#include "platform/events/WindowEvent.h"
#include "platform/events/Event.h"

#include "system/timer.h"

#include "rendering/RenderContext.h"

#include "graphics/Material.h"
#include "graphics/Shader.h"

#include "scene/spatial/Scene.h"
#include "render/Renderer.h"

class CrawlerGame : public fw::game
{
public:
	CrawlerGame() = default;
	virtual ~CrawlerGame() = default;

	bool on_game_startup() override;
	void on_game_shutdown() override;

	bool update(f64 deltaTime) override;

	void on_event(Event& e) override;

	fw::game::options get_startup_options() override;
	fw::window::state get_window_startup_state() override;
private:
	bool on_window_resize(WindowResizeEvent& e);

	void update_impl(float dt);

	void debug_setup();
private:
	// testing
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<mygui::Context> m_imGui;
	Entity* m_camera;
};