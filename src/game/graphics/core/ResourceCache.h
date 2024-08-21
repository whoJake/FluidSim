#pragma once

#include "vkcommon.h"
#include "ShaderModule.h"
#include "Framebuffer.h"
#include "DescriptorSetLayout.h"

namespace vk
{

class Device;

struct ResourceCacheState
{
    std::unordered_map<size_t, ShaderModule> shaderModules;
    std::unordered_map<size_t, Framebuffer> framebuffers;
    std::unordered_map<size_t, DescriptorSetLayout> descriptorSetLayouts;
};

class ResourceCache
{
public:
    ResourceCache(Device& device);

    ResourceCache(const ResourceCache& other) = delete;
    ResourceCache(ResourceCache&& other) = delete;
    ResourceCache& operator=(const ResourceCache& other) = delete;
    ResourceCache& operator=(ResourceCache&& other) = delete;

    ShaderModule& request_shader_module(VkShaderStageFlagBits stage, const std::vector<uint8_t>& glslSource, const std::string& entryPoint);

    Framebuffer& request_framebuffer(const RenderTarget& renderTarget, const RenderPass& renderPass);

    DescriptorSetLayout& request_descriptor_set_layout(uint32_t setIndex, const std::vector<ShaderModule*>& shaderModules, const std::vector<ShaderResource>& resources);

    void clear_framebuffers();

    void clear();

    const ResourceCacheState& get_internal_state() const;

private:
    Device& m_device;

    ResourceCacheState m_state;

};

} // vk