#pragma once
#include "application/WindowedApplication.h"
#include "platform/events/WindowEvent.h"
#include "platform/events/Event.h"

#include "system/timer.h"

#include "rendering/RenderContext.h"

#include "graphics/Material.h"
#include "graphics/Shader.h"

#include "scene/spatial/Scene.h"
#include "render/SceneRenderer.h"
#include "render/Renderer.h"

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
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<Renderer> m_renderer;
	Entity* m_camera;
};