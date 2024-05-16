#pragma once

#include "vkcommon.h"
#include "RenderTarget.h"
#include "core/Device.h"
#include "core/FencePool.h"
#include "core/SemaphorePool.h"
#include "core/CommandPool.h"
#include "core/DescriptorSet.h"
#include "core/Buffer.h"

namespace vk
{

class RenderFrame
{
public:
    RenderFrame(Device&                         device,
                std::unique_ptr<RenderTarget>&& renderTarget,
                size_t                          threadCount   = 1);

    RenderFrame(const RenderFrame&) = delete;
    RenderFrame(RenderFrame&&) = delete;
    RenderFrame& operator=(const RenderFrame&) = delete;
    RenderFrame& operator=(RenderFrame&&) = delete;

    void reset();

    Device& get_device() const;

    RenderTarget& get_render_target();

    const RenderTarget& get_render_target_const() const;

    const FencePool& get_fence_pool() const;

    VkFence request_fence();

    const SemaphorePool& get_semaphore_pool() const;

    VkSemaphore request_semaphore();

    VkSemaphore request_semaphore_with_ownership();

    void release_owned_semaphore(VkSemaphore semaphore);

    CommandBuffer& request_command_buffer(const Queue&             queue,
                                          CommandBuffer::ResetMode resetMode,
                                          size_t                   threadIndex = 0,
                                          VkCommandBufferLevel     level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkDescriptorSet request_descriptor_set(const DescriptorSetLayout&                 layout,
                                           size_t                                     threadIndex = 0,
                                           const std::vector<VkDescriptorBufferInfo>& buffers = { },
                                           const std::vector<VkDescriptorImageInfo>&  images  = { });

    void clear_descriptors();

    Buffer allocate_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryusage);

    void update_descriptor_sets(size_t threadIndex = 0);

    void update_render_target(std::unique_ptr<RenderTarget>&& renderTarget);
private:
    std::vector<std::unique_ptr<CommandPool>>& get_command_pools(const Queue& queue, CommandBuffer::ResetMode resetMode);
private:
    Device& m_device;

    size_t m_threadCount;

    FencePool m_fencePool;
    SemaphorePool m_semaphorePool;

    std::unique_ptr<RenderTarget> m_swapchainRenderTarget;

    // Family index -> CommandPool per thread
    std::map<uint32_t, std::vector<std::unique_ptr<CommandPool>>> m_commandPools;

    // Hashed DescriptorPool
    std::vector<std::unique_ptr<std::unordered_map<size_t, DescriptorPool>>> m_descriptorPools;

    // Hashed DescriptorSet
    std::vector<std::unique_ptr<std::unordered_map<size_t, DescriptorSet>>> m_descriptorSets;
};

} // vk