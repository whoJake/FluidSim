#pragma once

#include "vkcommon.h"
#include "Resource.h"
#include "shader_parser/ShaderResource.h"

namespace vk
{

class Device;

class ShaderModule
{
public:
    ShaderModule(Device& device,
                 VkShaderStageFlagBits stage,
                 const std::vector<uint8_t>& glslSource,
                 const std::string& entryPoint);

    ShaderModule(const ShaderModule& other) = delete;
    ShaderModule(ShaderModule&& other) noexcept;
    ShaderModule& operator=(const ShaderModule& other) = delete;
    ShaderModule& operator=(ShaderModule&& other) = delete;

    const Device& get_device() const;

    size_t get_binary_size() const;
    const uint32_t* get_binary_data() const;

    const std::vector<ShaderResource>& get_resources() const;

    VkShaderStageFlagBits get_stage() const;

    const std::string& get_entry_point() const;

    size_t get_uid() const;
private:
    Device& m_device;
    size_t m_UID;
    VkShaderStageFlagBits m_stage{ };
    std::string m_entryPoint;
    std::vector<uint32_t> m_SPIRV;

    std::vector<ShaderResource> m_resources;
};

} // vk
