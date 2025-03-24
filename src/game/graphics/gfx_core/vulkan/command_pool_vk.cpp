#include "command_pool_vk.h"
#include "gfx_core/Driver.h"
#include "device_vk.h"

#ifdef GFX_SUPPORTS_VULKAN
namespace gfx
{

void command_pool_vk::initialise(device_state_vk* state)
{
    GFX_ASSERT(state, "Vk Device State must be supplied.");
    GFX_ASSERT(!m_pools, "Command pool has already been initialised.");
    GFX_ASSERT(!m_locks, "Command pool has already been initialised.");
    GFX_ASSERT(m_poolCount == 0, "Command pool has already been initialised.");

    m_state = state;
    m_poolCount = u64_cast(state->get_queue_family_count());
    m_pools = new VkCommandPool[m_poolCount];
    m_buffersActive = new u64[m_poolCount];
    m_locks = new std::mutex[m_poolCount];
    
    VkDevice device = m_state->device;

    for( u64 poolIdx = 0; poolIdx < m_poolCount; poolIdx++ )
    {
        m_buffersActive[poolIdx] = 0;

        VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        createInfo.queueFamilyIndex = u32_cast(poolIdx);
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        switch( vkCreateCommandPool(device, &createInfo, nullptr, &m_pools[poolIdx]) )
        {
        case VK_SUCCESS:
            break;
        default:
            // TODO
            break;
        }
    }
}

void command_pool_vk::shutdown()
{
    GFX_ASSERT(m_pools, "Command pool has not been initialised.");
    GFX_ASSERT(m_locks, "Command pool has not been initialised.");
    GFX_ASSERT(m_poolCount != 0, "Command pool has not been initialised.");

    VkDevice device = m_state->device;

    // TODO add some "nothing is allocating/freeing currently" checks.
    // On shutdown this is very unlikely but some asserts can't hurt.
    for( u64 poolIdx = 0; poolIdx < m_poolCount; poolIdx++ )
    {
        GFX_ASSERT(m_buffersActive[poolIdx] == 0, "Command pool idx {} contains {} unfreed Command buffers.", poolIdx, m_buffersActive[poolIdx]);
        vkDestroyCommandPool(device, m_pools[poolIdx], nullptr);
    }

    delete[] m_pools;
    delete[] m_buffersActive;
    delete[] m_locks;
}

VkCommandBuffer command_pool_vk::allocate_buffer(u32 family_index)
{
    GFX_ASSERT(family_index < m_poolCount, "Family Index is invalid.");

    VkDevice device = m_state->device;

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = m_pools[family_index];
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer retval{ VK_NULL_HANDLE };

    {
        std::unique_lock<std::mutex> lock(m_locks[family_index]);

        VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &retval);
        switch( result )
        {
        case VK_SUCCESS:
            m_buffersActive[family_index]++;
            break;
        default:
            vkFreeCommandBuffers(device, m_pools[family_index], 1, &retval);
            retval = VK_NULL_HANDLE;
            break;
        }
    }

    return retval;
}

VkCommandBuffer command_pool_vk::allocate_buffer_by_flags(VkQueueFlags flags)
{
    return allocate_buffer(m_state->get_family_index_by_flags(flags));
}

void command_pool_vk::free_buffer(VkCommandBuffer buffer, u32 family_index)
{
    GFX_ASSERT(buffer != VK_NULL_HANDLE, "Command buffer is invalid.");
    GFX_ASSERT(family_index < m_poolCount, "Family Index is invalid.");

    VkDevice device = m_state->device;
    {
        std::unique_lock<std::mutex> lock(m_locks[family_index]);
        vkFreeCommandBuffers(device, m_pools[family_index], 1, &buffer);
        m_buffersActive[family_index]--;
    }
}

void command_pool_vk::free_buffer_by_flags(VkCommandBuffer buffer, VkQueueFlags flags)
{
    free_buffer(buffer, m_state->get_family_index_by_flags(flags));
}

} // gfx
#endif // GFX_SUPPORTS_VULKAN