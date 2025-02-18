#include "glsl_compiler.h"
#include "channels.h"

// Some of these had to have glslang brought in as dependency instead of from VulkanSDK :(
#include "StandAlone/DirStackFileIncluder.h"
#include "SPIRV/GLSL.std.450.h"
#include "SPIRV/GlslangToSpv.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Public/ResourceLimits.h"

glsl_compiler::glsl_compiler(glslang::EShTargetLanguage target,
                             glslang::EShTargetLanguageVersion targetVersion) :
    m_lang(target),
    m_langVer(targetVersion)
{
    glslang::InitializeProcess();
}

glsl_compiler::~glsl_compiler()
{
    glslang::FinalizeProcess();
}

bool glsl_compiler::compile(gfx::shader_stage_flag_bits stage,
                            const char* source_str,
                            const char* entry_point,
                            std::vector<u32>* out_spirv)
{
    EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules | EShMsgEnhanced);
    EShLanguage language = convert_stage(stage);

    glslang::TShader shader(language);
    shader.setEnvInput(glslang::EShSourceGlsl, language, glslang::EShClientVulkan, 100);
    shader.setStringsWithLengths(&source_str, nullptr, 1);
    shader.setEntryPoint(entry_point);
    shader.setSourceEntryPoint(entry_point);
    shader.setEnvTarget(m_lang, m_langVer);

    DirStackFileIncluder includeStack;
    includeStack.pushExternalLocalDirectory("res/shaders");

    // Needs include stack?
    if( !shader.parse(GetDefaultResources(), 100, false, messages, includeStack) )
    {
        if( shader.getInfoLog() && *shader.getInfoLog() )
            COMPILER_ERROR(shader.getInfoLog());
        if( shader.getInfoDebugLog() && *shader.getInfoDebugLog() )
            COMPILER_ERROR(shader.getInfoDebugLog());
        return false;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if( !program.link(messages) )
    {
        if( program.getInfoLog() && *program.getInfoLog() )
            COMPILER_ERROR(program.getInfoLog());
        if( program.getInfoDebugLog() && *program.getInfoDebugLog() )
            COMPILER_ERROR(program.getInfoDebugLog());
        return false;
    }

    glslang::TIntermediate* intermediate = program.getIntermediate(language);
    if( !intermediate )
    {
        COMPILER_ERROR("Failed to retrieve intermediate code");
        return false;
    }

    spv::SpvBuildLogger build_log;
    glslang::GlslangToSpv(*intermediate, *out_spirv, &build_log);

    std::string build_messages = build_log.getAllMessages();
    if( !build_messages.empty() )
        COMPILER_INFO("{}", build_messages.c_str());

    return true;
}

EShLanguage glsl_compiler::convert_stage(gfx::shader_stage_flag_bits stage)
{
    switch( stage )
    {
    case gfx::SHADER_STAGE_VERTEX:
        return EShLangVertex;
    case gfx::SHADER_STAGE_GEOMETRY:
        return EShLangGeometry;
    case gfx::SHADER_STAGE_FRAGMENT:
        return EShLangFragment;
    case gfx::SHADER_STAGE_COMPUTE:
        return EShLangCompute;
    default:
        COMPILER_FATAL("Shader stage is not supported by glsl_compiler.");
        return EShLangVertex;
    }
}