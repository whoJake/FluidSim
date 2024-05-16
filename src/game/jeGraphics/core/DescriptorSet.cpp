#include "DescriptorSet.h"

namespace vk
{

DescriptorSet::DescriptorSet(Device&                             device,
                             DescriptorPool&                     pool,
                             std::vector<VkDescriptorBufferInfo> bufferInfos,
                             std::vector<VkDescriptorImageInfo>  imageInfos) :
    Resource(pool.allocate(), device),
    m_pool(pool),
    m_bufferInfos(bufferInfos),
    m_imageInfos(imageInfos)
{ }

DescriptorSet::DescriptorSet(DescriptorSet&& other) :
    Resource(other.m_handle, other.m_device),
    m_pool(other.m_pool),
    m_bufferInfos(std::move(other.m_bufferInfos)),
    m_imageInfos(std::move(other.m_imageInfos))
{
    other.m_handle = VK_NULL_HANDLE;
}

const DescriptorSetLayout& DescriptorSet::get_descriptor_set_layout() const
{
    return m_pool.get_layout();
}

} // vk