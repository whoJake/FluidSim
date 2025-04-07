#pragma once
#include "dt/vector.h"

namespace gfx
{

class texture_view;
class command_list;

/// <summary>
/// Render Phase : Section of rendering which renders to a given set of outputs.
/// </summary>
class render_phase
{
public:
    using record_func = std::function<void(command_list*)>;
public:
    render_phase(std::string name);
    ~render_phase();

    void register_record_function(const char* name, record_func function);

    void begin(command_list* list, const dt::vector<texture_view*>& color_outputs, texture_view* depth_output = nullptr);
    void record();
    void end();

    void reset();
    bool is_active() const;
private:
    std::string m_name;
    command_list* m_activeCmdList;
    dt::vector<std::pair<const char*, record_func>> m_recordFunctions;
};

} // gfx