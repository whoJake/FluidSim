#pragma once

namespace gfx
{

// Program loading
class program;
struct compiled_program;
struct compiled_shader;
struct compiled_pass;

class loaders
{
public:
    loaders() = delete;
    ~loaders() = delete;
    DELETE_COPY(loaders);
    DELETE_MOVE(loaders);

    static bool save(const char* filename,
                     const char* program_name,
                     const compiled_program& program,
                     const std::vector<compiled_pass>& pass_descriptions,
                     const std::vector<compiled_shader>& shader_descriptions,
                     const std::vector<const char*>& shader_entry_points,
                     const std::vector<std::vector<u32>>& shader_datas);

    static bool load(const char* filename,
                     program* out_program);
};

} // gfx