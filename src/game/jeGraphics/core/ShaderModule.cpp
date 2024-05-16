#include "ShaderModule.h"
#include "Device.h"
#include "shader_parser/SPIRVReflection.h"
#include "shader_parser/GLSLCompiler.h"

namespace vk
{

#pragma warning(disable: 6262) // Function uses a lot of stack data apparently, I think its the spirv reflection (~16700 bytes)

ShaderModule::ShaderModule(Device&                     device,
                           VkShaderStageFlagBits       stage,
                           const std::vector<uint8_t>& glslSource,
                           const std::string&          entryPoint) :
    m_device(device),
    m_UID(0),
    m_stage(stage),
    m_entryPoint(entryPoint),
    m_SPIRV(),
    m_resources()
{
    GLSLCompiler compiler(m_device.get_log());
    std::string compilerInfoLog;
    bool compileSuccess = compiler.compile_to_spirv(stage, glslSource, entryPoint, &m_SPIRV, &compilerInfoLog);

    if( !compileSuccess )
    {
        JCLOG_ERROR(get_device().get_log(), "{}", compilerInfoLog.c_str());
        QUITFMT("Shader compilation failed. See above for details.");
    }

    SPIRVReflection reflection(m_SPIRV, m_stage);
    reflection.reflect_shader_resources(&m_resources);

    std::hash<std::string> strhash{ };
    m_UID = strhash(std::string(m_SPIRV.begin(), m_SPIRV.end()));
}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept:
    m_device(other.m_device),
    m_UID(other.m_UID),
    m_stage(other.m_stage),
    m_entryPoint(other.m_entryPoint),
    m_SPIRV(other.m_SPIRV),
    m_resources(other.m_resources)
{
    other.m_stage = { };
}

const Device& ShaderModule::get_device() const
{
    return m_device;
}

size_t ShaderModule::get_binary_size() const
{
    return m_SPIRV.size() * 4;
}

const uint32_t* ShaderModule::get_binary_data() const
{
    return m_SPIRV.data();
}

const std::vector<ShaderResource>& ShaderModule::get_resources() const
{
    return m_resources;
}

VkShaderStageFlagBits ShaderModule::get_stage() const
{
    return m_stage;
}

const std::string& ShaderModule::get_entry_point() const
{
    return m_entryPoint;
}

size_t ShaderModule::get_uid() const
{
    return m_UID;
}

} // vk