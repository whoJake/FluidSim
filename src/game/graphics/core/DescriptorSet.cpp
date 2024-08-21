#include "DescriptorSet.h"
#include "Device.h"

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

void DescriptorSet::write_buffers(VkDescriptorType type, u32 dstBinding, u32 count) const
{
    std::vector<VkWriteDescriptorSet> writes;

    for( u32 idx = 0; idx < count; idx++ )
    {
        u32 binding = dstBinding + idx;

        VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        write.dstSet = m_handle;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = &m_bufferInfos[idx];

        writes.push_back(write);
    }

    vkUpdateDescriptorSets(get_device().get_handle(), u32_cast(writes.size()), writes.data(), 0, nullptr);
}

} // vk