#pragma once

#include "vkcommon.h"
#include "ShaderModule.h"
#include "Resource.h"

namespace vk
{

class Device;
class DescriptorSetLayout;

class PipelineLayout : public Resource<VkPipelineLayout>
{
public:
    PipelineLayout(Device& device, const std::vector<ShaderModule*>& shaderModules);
    ~PipelineLayout() override;

    PipelineLayout(const PipelineLayout&) = delete;
    PipelineLayout(PipelineLayout&&);
    PipelineLayout& operator=(const PipelineLayout&) = delete;
    PipelineLayout& operator=(PipelineLayout&&) = delete;

    const std::vector<ShaderModule*>& get_shader_modules() const;

    const std::vector<ShaderResource> get_resources(const ShaderResourceType& type = ShaderResourceType::All, VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;

    DescriptorSetLayout& get_descriptor_set_layout(uint32_t setIndex) const;

    u32 get_descriptor_set_count() const;
private:
    std::vector<ShaderModule*> m_shaderModules;

    std::unordered_map<std::string, ShaderResource> m_shaderResources;

    std::unordered_map<uint32_t, std::vector<ShaderResource>> m_shaderSets;

    std::vector<DescriptorSetLayout*> m_descriptorSetLayouts;
};

} // vk