#include "ResourceCache.h"
#include "Device.h"

namespace vk
{

ResourceCache::ResourceCache(Device& device) :
    m_device(device),
    m_state()
{ }

ShaderModule& ResourceCache::request_shader_module(VkShaderStageFlagBits stage, const std::vector<uint8_t>& glslSource, const std::string& entryPoint)
{
    return request_resource(m_device.get_log(),  m_state.shaderModules, m_device, stage, glslSource, entryPoint);
}

Framebuffer& ResourceCache::request_framebuffer(const RenderTarget& renderTarget, const RenderPass& renderPass)
{
    return request_resource(m_device.get_log(), m_state.framebuffers, m_device, renderTarget, renderPass);
}

DescriptorSetLayout& ResourceCache::request_descriptor_set_layout(uint32_t setIndex, const std::vector<ShaderModule*>& shaderModules, const std::vector<ShaderResource>& resources)
{
    return request_resource(m_device.get_log(), m_state.descriptorSetLayouts, m_device, setIndex, shaderModules, resources);
}

void ResourceCache::clear_framebuffers()
{
    m_state.framebuffers.clear();
}

void ResourceCache::clear()
{
    m_state.shaderModules.clear();
    m_state.descriptorSetLayouts.clear();
    m_state.framebuffers.clear();
}

const ResourceCacheState& ResourceCache::get_internal_state() const
{
    return m_state;
}

} // vk