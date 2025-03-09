#pragma once
#include "gfx_core/pipeline_state.h"
#include "dt/vector.h"
#include "spirv_cross/spirv_glsl.hpp"
#include "gfx_core/loaders/shader_loader.h"

struct shader_resource {
    std::string name;
    gfx::shader_resource_type type;
    uint32_t qualifiers;

    uint32_t set;
    uint32_t binding;
    uint32_t location;
    uint32_t inputAttachmentIndex;

    uint32_t vecSize;
    uint32_t columns;
    uint32_t arraySize;
    uint32_t offset;
    uint32_t stride;
    uint32_t constantID;
};

class reflector
{
public:
    reflector() = delete;
    ~reflector() = delete;

    static bool reflect(gfx::program_def* pProgram);
private:
    static bool reflect_pass(gfx::program_def* pProgram, u64 passIdx, dt::vector<dt::vector<shader_resource>>& shader_resources);

    static bool reflect_shader(gfx::shader_def* pShader, dt::vector<shader_resource>* out_resources);
    static bool parse_shader_resources(spirv_cross::CompilerGLSL& compiler,
                                       spirv_cross::ShaderResources& resources,
                                       dt::vector<shader_resource>* out_resources);
};