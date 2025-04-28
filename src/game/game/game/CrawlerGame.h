#pragma once

#include "base/game.h"

#include "platform/events/WindowEvent.h"
#include "platform/events/Event.h"

#include "implementations/ImGuiContext.h"

class CrawlerGame : public fw::game
{
public:
	CrawlerGame() = default;
	virtual ~CrawlerGame() = default;

	bool on_game_startup() override;
	void on_game_shutdown() override;

	bool update(f64 deltaTime) override;

	void on_event(Event& e) override;

	fw::window::state get_window_startup_state() override;
private:
	bool on_window_resize(WindowResizeEvent& e);

	void update_impl(float dt);

	void debug_setup();
private:
	// testing
	std::unique_ptr<mygui::Context> m_imGui;
	glm::vec3 m_position;
	glm::vec2 m_cursor;
};