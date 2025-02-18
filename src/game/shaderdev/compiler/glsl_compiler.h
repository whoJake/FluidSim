#pragma once
#include "gfx_core/types.h"
#include "glslang/Public/ShaderLang.h"

class glsl_compiler
{
public:
    glsl_compiler(glslang::EShTargetLanguage target = glslang::EShTargetLanguage::EShTargetSpv,
                  glslang::EShTargetLanguageVersion targetVersion = glslang::EShTargetLanguageVersion::EShTargetSpv_1_6);

    ~glsl_compiler();

    bool compile(gfx::shader_stage_flag_bits stage,
                 const char* source_str,
                 const char* entry_point,
                 std::vector<u32>* out_spirv);

private:
    glslang::EShTargetLanguage m_lang;
    glslang::EShTargetLanguageVersion m_langVer;

    static EShLanguage convert_stage(gfx::shader_stage_flag_bits stage);
};