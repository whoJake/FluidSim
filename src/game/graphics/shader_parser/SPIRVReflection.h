#pragma once

#include "vkcommon.h"
#include "spirv_cross/spirv_glsl.hpp"
#include "ShaderResource.h"

namespace vk
{

class SPIRVReflection
{
public:
    SPIRVReflection(std::vector<uint32_t>& spirv, VkShaderStageFlagBits stage);
    void reflect_shader_resources(std::vector<ShaderResource>* resourceList);
private:
    void parse_shader_resources(std::vector<ShaderResource>& resources);
    void parse_push_constants(std::vector<ShaderResource>& resources);
    void parse_specialization_constants(std::vector<ShaderResource>& resources);
private:
    spirv_cross::CompilerGLSL m_compiler;
    VkShaderStageFlagBits m_stage;
    spirv_cross::ShaderResources m_spirvResources;
};

} // vk