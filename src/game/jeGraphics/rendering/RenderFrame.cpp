#include "RenderFrame.h"

namespace vk
{

RenderFrame::RenderFrame(Device&                        device,
                        std::unique_ptr<RenderTarget>&& renderTarget,
                        size_t                          threadCount) :
    m_device(device),
    m_threadCount(threadCount),
    m_fencePool(device),
    m_semaphorePool(device),
    m_swapchainRenderTarget(std::move(renderTarget))
{
    // make buffer pool stuff here likely

    m_descriptorPools.push_back(std::make_unique<std::unordered_map<size_t, DescriptorPool>>());
    m_descriptorSets.push_back(std::make_unique<std::unordered_map<size_t, DescriptorSet>>());
}

void RenderFrame::reset()
{
    VkResult waitFenceResult = m_fencePool.wait();
    VK_CHECK(waitFenceResult, "Failed to reset RenderFrame as was unable to wait for fences.");

    m_fencePool.reset();

    for( auto& commandPoolMap : m_commandPools )
    {
        for( auto& commandPoolThreads : commandPoolMap.second )
        {
            commandPoolThreads->reset_pool();
        }
    }

    m_semaphorePool.reset();

    // Clear individual descriptor sets? I think not...
}

Device& RenderFrame::get_device() const
{
    return m_device;
}

RenderTarget& RenderFrame::get_render_target()
{
    return *m_swapchainRenderTarget;
}

const RenderTarget& RenderFrame::get_render_target_const() const
{
    return *m_swapchainRenderTarget;
}

const FencePool& RenderFrame::get_fence_pool() const
{
    return m_fencePool;
}

VkFence RenderFrame::request_fence()
{
    return m_fencePool.request_fence();
}

const SemaphorePool& RenderFrame::get_semaphore_pool() const
{
    return m_semaphorePool;
}

VkSemaphore RenderFrame::request_semaphore()
{
    return m_semaphorePool.request_semaphore();
}

VkSemaphore RenderFrame::request_semaphore_with_ownership()
{
    return m_semaphorePool.request_semaphore_with_ownership();
}

void RenderFrame::release_owned_semaphore(VkSemaphore semaphore)
{
    m_semaphorePool.release_owned_semaphore(semaphore);
}

CommandBuffer& RenderFrame::request_command_buffer(const Queue&             queue,
                                                   CommandBuffer::ResetMode resetMode,
                                                   size_t                   threadIndex,
                                                   VkCommandBufferLevel     level)
{
    std::vector<std::unique_ptr<CommandPool>>& commandPools = get_command_pools(queue, resetMode);
    return commandPools[threadIndex]->request_command_buffer(level);
}

VkDescriptorSet RenderFrame::request_descriptor_set(const DescriptorSetLayout&                 layout,
                                                    size_t                                     threadIndex,
                                                    const std::vector<VkDescriptorBufferInfo>& buffers,
                                                    const std::vector<VkDescriptorImageInfo>&  images)
{
    DescriptorPool& pool = request_resource(*m_descriptorPools[threadIndex], m_device, layout);
    DescriptorSet& set = request_resource(*m_descriptorSets[threadIndex], m_device, pool, buffers, images);

    // Apply writes?
    return set.get_handle();
}

void RenderFrame::clear_descriptors()
{
    for( auto& sets : m_descriptorSets )
    {
        sets->clear();
    }

    for( auto& pools : m_descriptorPools )
    {
        for( auto& pool : *pools )
        {
            pool.second.reset();
        }
    }
}

Buffer RenderFrame::allocate_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    return Buffer(get_device(), size, usage, memoryUsage);
}

void RenderFrame::update_descriptor_sets(size_t threadIndex)
{
}

void RenderFrame::update_render_target(std::unique_ptr<RenderTarget>&& renderTarget)
{
    m_swapchainRenderTarget = std::move(renderTarget);
}

std::vector<std::unique_ptr<CommandPool>>& RenderFrame::get_command_pools(const Queue& queue, CommandBuffer::ResetMode resetMode)
{
    auto it = m_commandPools.find(queue.get_family_index());

    if( it != m_commandPools.end() )
    {
        if( it->second[0]->get_reset_mode() != resetMode )
        {
            get_device().wait_idle();
            m_commandPools.erase(it);
        }
        else
        {
            return it->second;
        }
    }

    std::vector<std::unique_ptr<CommandPool>> createdPools;
    for( size_t i = 0; i < m_threadCount; i++ )
    {
        createdPools.push_back(std::make_unique<CommandPool>(get_device(), queue.get_family_index(), this, i, resetMode));
    }

    auto insertIt = m_commandPools.emplace(queue.get_family_index(), std::move(createdPools));

    if( !insertIt.second )
    {
        QUITFMT("Failed to insert CommandPools for family index {}.", queue.get_family_index());
    }

    it = insertIt.first;
    return it->second;
}

} // vk