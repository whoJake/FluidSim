#pragma once

#include "glslang/Public/ShaderLang.h"
#include "vkcommon.h"

namespace vk
{

class GLSLCompiler
{
public:
    GLSLCompiler(const jclog::Log&                 log,
                 glslang::EShTargetLanguage        targetLang           = glslang::EShTargetLanguage::EShTargetSpv,
                 glslang::EShTargetLanguageVersion targetLangVer        = glslang::EShTargetLanguageVersion::EShTargetSpv_1_5) :
        m_log(log),
        m_targetLang(targetLang),
        m_targetLangVer(targetLangVer)
    { }

    /**
    * @brief Compile GLSL source code into SPIRV
    * @param "stage" Shader stage that the source code belongs to
    * @param "glslSource" The GLSL source code to be compiled
    * @param "glslSourceDir" Directory that the GLSL source file came from
    * @param "entryPoint" Function name of the entry point of the shader
    * @param[out] "spirv" (out) Compiled SPIRV code
    * @param[out] "infoLog" (out) String of all debug info logging
    */
    bool compile_to_spirv(VkShaderStageFlagBits stage,
        const std::vector<uint8_t>&             glslSource,
        const std::string&                      entryPoint,
        std::vector<uint32_t>*                  spirv,
        std::string*                            infoLog);

private:
    const jclog::Log& m_log;
    glslang::EShTargetLanguage m_targetLang;
    glslang::EShTargetLanguageVersion m_targetLangVer;

private:
    EShLanguage get_shader_language(VkShaderStageFlagBits stage) const;
};

} // vk