#include "DescriptorSetLayout.h"

#include "Device.h"
#include "shader_parser/ShaderResource.h"


namespace vk
{

DescriptorSetLayout::DescriptorSetLayout(Device&                            device,
                                         uint32_t                           setIndex,
                                         const std::vector<ShaderModule*>&  shaderModules,
                                         const std::vector<ShaderResource>& resources) :
    Resource(VK_NULL_HANDLE, device),
    m_setIndex(setIndex),
    m_shaderModules(shaderModules)
{
    for( const ShaderResource& resource : resources )
    {
        if( resource.type == ShaderResourceType::Input
            || resource.type == ShaderResourceType::Output
            || resource.type == ShaderResourceType::SpecializationConstant
            || resource.type == ShaderResourceType::PushConstant )
        {
            // These resource types don't contain a binding point and arent a part of descriptor set layouts.
            continue;
        }

        VkDescriptorSetLayoutBinding binding{ };
        binding.binding = resource.binding;
        binding.stageFlags = resource.stages;
        binding.descriptorCount = resource.arraySize;
        binding.descriptorType = get_descriptor_type(resource.type);

        m_bindings.push_back(binding);
        m_bindingMap.emplace(resource.binding, binding);
        m_resourceToBindingMap.emplace(resource.name, resource.binding);
    }

    VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    createInfo.bindingCount = to_u32(m_bindings.size());
    createInfo.pBindings = m_bindings.data();

    VkResult result = vkCreateDescriptorSetLayout(get_device().get_handle(), &createInfo, nullptr, &m_handle);
    VK_CHECK(result, "Failed to create DescriptorSetLayout.");

    calculate_hash();
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
    Resource(other.m_handle, other.m_device),
    m_hash(other.m_hash),
    m_setIndex(other.m_setIndex),
    m_bindings(other.m_bindings),
    m_bindingMap(other.m_bindingMap),
    m_resourceToBindingMap(other.m_resourceToBindingMap),
    m_shaderModules(other.m_shaderModules)
{
    other.m_handle = VK_NULL_HANDLE;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(get_device().get_handle(), m_handle, nullptr);
}

uint32_t DescriptorSetLayout::get_index() const
{
    return m_setIndex;
}

const std::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayout::get_bindings() const
{
    return m_bindings;
}

const std::vector<ShaderModule*>& DescriptorSetLayout::get_shader_modules() const
{
    return m_shaderModules;
}

size_t DescriptorSetLayout::get_hash() const
{
    return m_hash;
}

void DescriptorSetLayout::calculate_hash()
{
    for( VkDescriptorSetLayoutBinding& binding : m_bindings )
    {
        hash_combine(m_hash, binding.binding);
        hash_combine(m_hash, binding.descriptorCount);
        hash_combine(m_hash, binding.descriptorType);
        hash_combine(m_hash, binding.stageFlags);
    }
}

} // vk