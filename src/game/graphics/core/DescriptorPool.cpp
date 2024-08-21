#include "DescriptorPool.h"
#include "Device.h"
#include "DescriptorSet.h"

namespace vk
{

DescriptorPool::DescriptorPool(Device&                    device,
                               const DescriptorSetLayout& layout,
                               uint32_t                   setsPerPool) :
    m_device(device),
    m_layout(&layout),
    m_setsPerPool(setsPerPool),
    m_currentPoolIndex(0)
{ }

DescriptorPool::~DescriptorPool()
{
    for( VkDescriptorPool pool : m_pools )
    {
        vkDestroyDescriptorPool(m_device.get_handle(), pool, nullptr);
    }
}

const DescriptorSetLayout& DescriptorPool::get_layout() const
{
    return *m_layout;
}

void DescriptorPool::reset()
{
    for( VkDescriptorPool pool : m_pools )
    {
        vkResetDescriptorPool(m_device.get_handle(), pool, 0);
    }

    // Reset all pool sizes to 0
    std::fill(m_poolsSetCount.begin(), m_poolsSetCount.end(), 0);

    m_setToPoolIndex.clear();
    m_currentPoolIndex = 0;
}

VkDescriptorSet DescriptorPool::allocate()
{
    // Search for the next free pool.
    m_currentPoolIndex = get_next_free_pool(m_currentPoolIndex);

    VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    allocInfo.descriptorPool = m_pools[m_currentPoolIndex];
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_layout->get_handle();

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkResult result = vkAllocateDescriptorSets(m_device.get_handle(), &allocInfo, &descriptorSet);
    VK_CHECK(result, "Failed to allocate new DescriptorSet.");

    m_poolsSetCount[m_currentPoolIndex]++;
    m_setToPoolIndex.emplace(descriptorSet, m_currentPoolIndex);
    
    return descriptorSet;
}

VkResult DescriptorPool::free(VkDescriptorSet set)
{
    auto it = m_setToPoolIndex.find(set);
    if( it == m_setToPoolIndex.end() )
    {
        // Code error or set is not in this pool.
        return VK_INCOMPLETE;
    }

    uint32_t poolIndex = (*it).second;
    VkResult result = vkFreeDescriptorSets(m_device.get_handle(), m_pools[poolIndex], 1, &set);
    VK_CHECK(result, "Failed to free DescriptorSet.");

    m_poolsSetCount[poolIndex]--;
    m_setToPoolIndex.erase(it);

    // Set our index to pool we just freed.
    m_currentPoolIndex = poolIndex;

    return result;
}

uint32_t DescriptorPool::get_next_free_pool(uint32_t searchIndex)
{
    // Create new pool if we've overflowed
    if( m_pools.size() <= searchIndex )
    {
        VkDescriptorPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        createInfo.poolSizeCount = to_u32(m_poolSizes.size());
        createInfo.pPoolSizes = m_poolSizes.data();
        createInfo.maxSets = m_setsPerPool;
        createInfo.flags = 0;

        VkDescriptorPool newHandle = VK_NULL_HANDLE;
        VkResult result = vkCreateDescriptorPool(m_device.get_handle(), &createInfo, nullptr, &newHandle);

        VK_CHECK(result, "Failed to create new DescriptorPool");

        m_poolsSetCount.push_back(0);
        m_pools.push_back(newHandle);

        return searchIndex;
    }

    if( m_poolsSetCount[searchIndex] < m_setsPerPool )
    {
        return searchIndex;
    }

    // Check next pool
    return get_next_free_pool(++searchIndex);
}

} // vk