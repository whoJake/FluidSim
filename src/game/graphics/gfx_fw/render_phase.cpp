#include "render_phase.h"
#include "gfx_core/gfxdefines.h"
#include "gfx_core/Driver.h"

namespace gfx
{

render_phase::render_phase(std::string name) :
    m_name(name),
    m_activeCmdList(nullptr),
    m_recordFunctions()
{ }

render_phase::~render_phase()
{
    GFX_ASSERT(!is_active(), "Destroying render phase whilst its still active.");
}

void render_phase::register_record_function(const char* name, record_func function)
{
    m_recordFunctions.push_back(std::make_pair(name, function));
}

void render_phase::begin(command_list* list, const dt::vector<texture_view*>& color_outputs, texture_view* depth_output)
{
    std::vector<texture_view*> cpy_colors(color_outputs.size());
    for( u64 i = 0; i < color_outputs.size(); i++ )
    {
        cpy_colors[i] = color_outputs[i];
    }

    GFX_CALL(begin_rendering, list, cpy_colors.data(), u32_cast(color_outputs.size()), depth_output);
    m_activeCmdList = list;
}

void render_phase::record()
{
    GFX_ASSERT(is_active(), "Cannot record render phase when it has not been started. Has begin() been called?");

    for( std::pair<const char*, record_func>& recorders : m_recordFunctions )
    {
        recorders.second(m_activeCmdList);
    }
}

void render_phase::end()
{
    GFX_CALL(end_rendering, m_activeCmdList);
    m_activeCmdList = nullptr;
}

void render_phase::reset()
{
    GFX_ASSERT(!is_active(), "Resetting render phase that is still active.");
    m_recordFunctions.resize(0);
}

bool render_phase::is_active() const
{
    return m_activeCmdList;
}

} // gfx