#pragma once
#include <thread>
#include "context.h"

namespace gfx
{

class context_mgr
{
public:
    static void initialise();
    static void shutdown();

    static graphics_context* get_graphics_context();

    static void begin_frame();
    static void end_frame();
private:
    static context_mgr* sm_instance;
    static thread_local graphics_context* sm_graphicsContext;

public:
    ~context_mgr() = default;
private:
    context_mgr() = default;
private:
    u32 m_renderWorkerCount;
};

} // gfx