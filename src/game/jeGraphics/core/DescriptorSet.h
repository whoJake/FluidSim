#pragma once

#include "vkcommon.h"
#include "Resource.h"
#include "DescriptorPool.h"

namespace vk
{

class Device;

class DescriptorSet : public Resource<VkDescriptorSet>
{
public:
    DescriptorSet(Device&                             device,
                  DescriptorPool&                     pool,
                  std::vector<VkDescriptorBufferInfo> bufferInfos = {},
                  std::vector<VkDescriptorImageInfo>  imageInfos  = {});

    // Handle is dealt with by the DescriptorPool so nothing happens in here.
    ~DescriptorSet() = default;

    DescriptorSet(DescriptorSet&&);
    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(DescriptorSet&&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;

    const DescriptorSetLayout& get_descriptor_set_layout() const;

    void write_buffers(VkDescriptorType type, u32 dstBinding = 0, u32 count = 1) const;
private:
    DescriptorPool& m_pool;

    std::vector<VkDescriptorBufferInfo> m_bufferInfos;
    std::vector<VkDescriptorImageInfo> m_imageInfos;
};

} // vk