#pragma once

#include "app.h"
#include "../platform/window.h"
#include "system/timer.h"
#include "base/scaffold.h"

namespace fw
{

enum class game_exitcodes
{
    failure_during_update = 10,
};

#define EXIT_UPDATE_FAILURE EXITCODE(::fw::game_exitcodes::failure_during_update)

class game : public app
{
public:
    // MAKEGPARAM(headless);
    // MAKEGPARAM(no_graphics);

    game() = default;
    virtual ~game() = default;

    DELETE_MOVE(game);
    DELETE_COPY(game);

    i32 app_main() override final;

    bool on_startup() override final;
    void on_shutdown() override final;

    virtual void setup_startup_graph(scaffold_startup_node& parent);
    virtual void setup_update_graph(scaffold_update_node& parent);
    virtual void setup_shutdown_graph(scaffold_shutdown_node& parent);

    void set_should_close();

    window& get_window();
    const window& get_window() const;

    virtual bool on_game_startup();
    virtual void on_game_shutdown();

    virtual void on_event(Event& e);

    virtual window::state get_window_startup_state() = 0;
private:
    const char* m_name{ nullptr };

    std::unique_ptr<window> m_window{ nullptr };

    sys::moment m_lastUpdateTime;
    bool m_shouldClose{ false };
};

} // fw