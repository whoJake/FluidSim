#pragma once
#include "data/program_desc.h"
#include "pugi_include.h"
#include "shadev_channels.h"

#define ENUM_PARSE(enum_base, enum_value, value) if (!strcmp(value, #enum_value)) return enum_base::enum_value

class parser
{
public:
    parser() = delete;
    ~parser() = delete;

    static bool parse_program(const char* filename, shader_program_file* parsed_program);
private:
    static bool parse_program(pugi::xml_node root, shader_program_file* program);
    static bool parse_pass(pugi::xml_node root, shader_pass_desc* pass);
    static bool parse_pass_pso(pugi::xml_node root, gfx::pipeline_state* pso);

    static bool parse_shader(pugi::xml_node root, shader_file_desc* shader);
};

inline static gfx::format parse_format(const char* value)
{
    ENUM_PARSE(gfx::format, R8G8B8A8_SRGB, value);
    ENUM_PARSE(gfx::format, R8_UNORM, value);
    ENUM_PARSE(gfx::format, R8_SNORM, value);
    ENUM_PARSE(gfx::format, R8_UINT, value);
    ENUM_PARSE(gfx::format, R8_SINT, value);
    ENUM_PARSE(gfx::format, R8_SRGB, value);
    ENUM_PARSE(gfx::format, R8G8_UNORM, value);
    ENUM_PARSE(gfx::format, R8G8_SNORM, value);
    ENUM_PARSE(gfx::format, R8G8_UINT, value);
    ENUM_PARSE(gfx::format, R8G8_SINT, value);
    ENUM_PARSE(gfx::format, R8G8_SRGB, value);
    ENUM_PARSE(gfx::format, R8G8B8_UNORM, value);
    ENUM_PARSE(gfx::format, R8G8B8_SNORM, value);
    ENUM_PARSE(gfx::format, R8G8B8_UINT, value);
    ENUM_PARSE(gfx::format, R8G8B8_SINT, value);
    ENUM_PARSE(gfx::format, R8G8B8_SRGB, value);
    ENUM_PARSE(gfx::format, R8G8B8A8_UNORM, value);
    ENUM_PARSE(gfx::format, R8G8B8A8_SNORM, value);
    ENUM_PARSE(gfx::format, R8G8B8A8_UINT, value);
    ENUM_PARSE(gfx::format, R8G8B8A8_SINT, value);
    ENUM_PARSE(gfx::format, R8G8B8A8_SRGB, value);
    ENUM_PARSE(gfx::format, A2B10G10R10_UNORM, value);

    SHADEV_ASSERT(false, "Failed to parse enum gfx::format with value {}", value);
    return gfx::format::UNDEFINED;
}