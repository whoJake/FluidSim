#pragma once
#include "platform/window.h"
#include "system/timer.h"

#include "gfx_core/descriptor_pool.h"
#include "gfx_fw/context.h"

namespace mygui
{

class Context
{
public:
    Context(fw::window* glfwWindow);
    ~Context();

    void begin_frame();
    void end_frame();

    void render(gfx::graphics_context& context);
private:
    fw::window* m_window;
    gfx::descriptor_pool m_pool;
    gfx::descriptor_table_desc m_desc;
    sys::moment m_frameStart;
};

} // mygui