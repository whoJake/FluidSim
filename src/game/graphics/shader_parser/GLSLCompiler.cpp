#include "GLSLCompiler.h"

// Some of these had to have glslang brought in as dependency instead of from VulkanSDK :(
#include "StandAlone/DirStackFileIncluder.h"
#include "SPIRV/GLSL.std.450.h"
#include "SPIRV/GlslangToSpv.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Public/ResourceLimits.h"

namespace vk
{

bool GLSLCompiler::compile_to_spirv(VkShaderStageFlagBits       stage,
                                    const std::vector<uint8_t>& glslSource,
                                    const std::string&          entryPoint,
                                    std::vector<uint32_t>*      spirv,
                                    std::string*                infoLog)
{
    glslang::InitializeProcess();

    EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules | EShMsgEnhanced);
    EShLanguage language = get_shader_language(stage);
    std::string source = std::string(glslSource.begin(), glslSource.end());

    const char* fileNameList[1] = { "" };
    const char* shaderSource = reinterpret_cast<const char*>(source.data());

    glslang::TShader shader(language);
    shader.setStringsWithLengthsAndNames(&shaderSource, nullptr, fileNameList, 1); // ?
    shader.setEntryPoint(entryPoint.c_str());
    shader.setSourceEntryPoint(entryPoint.c_str());
    shader.setEnvTarget(m_targetLang, m_targetLangVer);

    DirStackFileIncluder includeStack;
    includeStack.pushExternalLocalDirectory("res/shaders");

    bool parseSuccess = shader.parse(GetDefaultResources(), 100, false, messages, includeStack);
    if (!parseSuccess)
    {
        *infoLog = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
        return false;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    bool linkSuccess = program.link(messages);
    if (!linkSuccess)
    {
        *infoLog = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        return false;
    }

    if (shader.getInfoLog())
        *infoLog += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";

    if (program.getInfoLog())
        *infoLog += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog()) + "\n";

    glslang::TIntermediate* intermediate = program.getIntermediate(language);
    if (!intermediate)
    {
        *infoLog += "Failed to get shared intermediate code\n";
        return false;
    }

    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*intermediate, *spirv, &logger);

    *infoLog += logger.getAllMessages() + "\n";

    glslang::FinalizeProcess();

    return true;
}

inline EShLanguage GLSLCompiler::get_shader_language(VkShaderStageFlagBits stage) const
{
    switch( stage )
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
        return EShLangVertex;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
        return EShLangGeometry;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
        return EShLangFragment;
    case VK_SHADER_STAGE_COMPUTE_BIT:
        return EShLangCompute;
    default:
        VULKAN_WARN("Shader langauge not implemented yet, defaulting to vertex shader.");
        return EShLangVertex;
    }
}

} // vk