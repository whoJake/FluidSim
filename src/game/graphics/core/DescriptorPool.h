#pragma once

#include "vkcommon.h"
#include "DescriptorSetLayout.h"

namespace vk
{

class Device;
class DescriptorSet;

class DescriptorPool
{
public:
    DescriptorPool(Device&                    device,
                   const DescriptorSetLayout& layout,
                   uint32_t                   setsPerPool = 16);

    ~DescriptorPool();

    const DescriptorSetLayout& get_layout() const;

    void reset();

    VkDescriptorSet allocate();

    VkResult free(VkDescriptorSet set);
private:
    uint32_t get_next_free_pool(uint32_t startIndex);

private:
    Device& m_device;

    const DescriptorSetLayout* m_layout;

    uint32_t m_setsPerPool;

    uint32_t m_currentPoolIndex;
    std::vector<VkDescriptorPool> m_pools;
    std::vector<uint32_t> m_poolsSetCount;

    std::vector<VkDescriptorPoolSize> m_poolSizes;

    std::unordered_map<VkDescriptorSet, uint32_t> m_setToPoolIndex;
};

} // vk

// hasher
namespace std
{

template<>
struct hash<vk::DescriptorPool>
{
    size_t operator()(const vk::DescriptorPool& pool) const
    {
        return pool.get_layout().get_hash();
    }
};

} // std