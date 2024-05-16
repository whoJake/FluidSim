#include "PipelineLayout.h"
#include "Device.h"
#include "DescriptorSetLayout.h"

namespace vk
{

PipelineLayout::PipelineLayout(Device& device, const std::vector<ShaderModule*>& shaderModules) :
    Resource(VK_NULL_HANDLE, device),
    m_shaderModules(shaderModules)
{

    // Get all resources out of modules and note the stages they're used in.
    for( auto* module : shaderModules )
    {
        for( const auto& resource : module->get_resources() )
        {
            std::string key = resource.name;

            // Input and Output can have same name so change name str.
            if( resource.type == ShaderResourceType::Input
                || resource.type == ShaderResourceType::Output )
            {
                key += "_" + std::to_string(resource.stages);
            }

            auto it = m_shaderResources.find(key);

            if( it != m_shaderResources.end() )
            {
                // Append stage
                it->second.stages |= resource.stages;
            }
            else
            {
                // Add resource
                m_shaderResources.emplace(key, resource);
            }

        }
    }

    // Sort resources into sets inside m_shaderSets.
    for( auto& it : m_shaderResources )
    {
        auto& resource = it.second;

        // Find set index.
        auto it2 = m_shaderSets.find(resource.set);

        if( it2 != m_shaderSets.end() )
        {
            // Add resource to set.
            it2->second.push_back(resource);
        }
        else
        {
            // Create new set with index.
            m_shaderSets.emplace(resource.set, std::vector<ShaderResource>{ resource });
        }
    }

    // Create actual descriptor set layouts out of m_shaderSets.
    for( auto& it : m_shaderSets )
    {
        m_descriptorSetLayouts.push_back(&get_device().get_resource_cache().request_descriptor_set_layout(it.first, m_shaderModules, it.second));
    }

    // Get set layout handles (in order) to put into vkCreatePipelineLayout.
    std::vector<VkDescriptorSetLayout> setLayouts;
    for( uint32_t i = 0; i < m_descriptorSetLayouts.size(); i++ )
    {
        setLayouts.push_back(m_descriptorSetLayouts[i]->get_handle());
    }

    // Push constants?
    std::vector<VkPushConstantRange> pushConstants;
    for( auto& pushResource : this->get_resources(ShaderResourceType::PushConstant) )
    {
        pushConstants.push_back({ pushResource.stages, pushResource.offset, pushResource.stride });
    }

    // vkCreatePipelineLayout yipee..
    VkPipelineLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    createInfo.setLayoutCount = to_u32(setLayouts.size());
    createInfo.pSetLayouts = setLayouts.data();
    createInfo.pushConstantRangeCount = to_u32(pushConstants.size());
    createInfo.pPushConstantRanges = pushConstants.data();

    VkResult result = vkCreatePipelineLayout(get_device().get_handle(), &createInfo, nullptr, &m_handle);
    VK_CHECK(result, "Failed to create PipelineLayout.");
}

PipelineLayout::PipelineLayout(PipelineLayout&& other) :
    Resource(other.get_handle(), other.get_device()),
    m_shaderModules(other.m_shaderModules),
    m_shaderResources(other.m_shaderResources),
    m_shaderSets(other.m_shaderSets),
    m_descriptorSetLayouts(other.m_descriptorSetLayouts)
{ 
    other.m_handle = VK_NULL_HANDLE;
}

PipelineLayout::~PipelineLayout()
{
    vkDestroyPipelineLayout(get_device().get_handle(), get_handle(), nullptr);
}

const std::vector<ShaderModule*>& PipelineLayout::get_shader_modules() const
{
    return m_shaderModules;
}

const std::vector<ShaderResource> PipelineLayout::get_resources(const ShaderResourceType& type, VkShaderStageFlagBits stage) const
{
    std::vector<ShaderResource> foundResources;

    for( auto& it : m_shaderResources )
    {
        auto& shaderResource = it.second;
        if( type == ShaderResourceType::All
            || type == shaderResource.type )
        {
            if( stage == VK_SHADER_STAGE_ALL
                || stage & shaderResource.stages )
            {
                foundResources.push_back(shaderResource);
            }
        }
    }

    return foundResources;
}

DescriptorSetLayout& PipelineLayout::get_descriptor_set_layout(uint32_t setIndex) const
{
    for( auto& pLayout : m_descriptorSetLayouts )
    {
        if( pLayout->get_index() == setIndex )
        {
            return *pLayout;
        }
    }
    
    QUITFMT("Pipeline layout doesn't contain set index {}.", setIndex);
}

} // vk