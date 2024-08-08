#pragma once
#include "application/WindowedApplication.h"
#include "platform/events/WindowEvent.h"
#include "platform/events/Event.h"

#include "system/timer.h"

#include "rendering/RenderContext.h"

#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/Blueprint.h"
#include "scene/BlueprintManager.h"

#include "render/SceneRenderer.h"

#include "graphics/Material.h"
#include "graphics/Shader.h"

class CrawlerGame : public WindowedApplication
{
public:
	CrawlerGame();
	~CrawlerGame();

	void on_app_startup() override;
	void on_app_shutdown() override;

	void update() override;

	void on_event(Event& e);
private:
	bool on_window_resize(WindowResizeEvent& e);

	void update_impl(float dt);

	void debug_setup();
private:
	sys::moment m_beginFrame;

	// testing
	std::unique_ptr<vk::RenderContext> m_context;
	std::unique_ptr<fw::Scene> m_scene;
	std::unique_ptr<fw::SceneRenderer> m_renderer;

	glm::vec3 m_position{ };
	glm::vec3 m_rotation{ 0.f, glm::radians(180.f), 0.f };
};