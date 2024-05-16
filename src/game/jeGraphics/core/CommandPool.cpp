#include "CommandPool.h"
#include "Device.h"

namespace vk
{

CommandPool::CommandPool(Device&                  device,
                         uint32_t                 queueFamilyIndex,
                         RenderFrame*             renderFrame,
                         size_t                   threadIndex,
                         CommandBuffer::ResetMode resetMode) :
    Resource(VK_NULL_HANDLE, device),
    m_queueFamilyIndex(queueFamilyIndex),
    m_renderFrame(renderFrame),
    m_threadIndex(threadIndex),
    m_resetMode(resetMode)
{
    VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    createInfo.queueFamilyIndex = queueFamilyIndex;
    
    switch( resetMode )
    {
    case CommandBuffer::ResetMode::AlwaysAllocate:
    case CommandBuffer::ResetMode::ResetIndividual:
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        break;
    case CommandBuffer::ResetMode::ResetPool:
        createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        break;
    default:
        QUITFMT("CommandBuffer ResetMode enum has not been implemented yet.");
    }

    VkResult createResult = vkCreateCommandPool(get_device().get_handle(), &createInfo, nullptr, &m_handle);
    VK_CHECK(createResult, "Failed to create CommandPool.");
}

CommandPool::~CommandPool()
{
    m_primaryCommandBuffers.clear();
    m_secondaryCommandBuffers.clear();

    vkDestroyCommandPool(get_device().get_handle(), get_handle(), nullptr);
}

CommandPool::CommandPool(CommandPool&& other) :
    Resource(other.m_handle, other.m_device),
    m_queueFamilyIndex(other.m_queueFamilyIndex),
    m_renderFrame(other.m_renderFrame),
    m_threadIndex(other.m_threadIndex),
    m_resetMode(other.m_resetMode)
{
    other.m_handle = VK_NULL_HANDLE;
}

CommandBuffer& CommandPool::request_command_buffer(VkCommandBufferLevel level)
{
    switch( level )
    {
    case VK_COMMAND_BUFFER_LEVEL_PRIMARY:
        if( m_activePrimaryCommandBuffers < m_primaryCommandBuffers.size() )
        {
            return *m_primaryCommandBuffers[m_activePrimaryCommandBuffers++];
        }

        m_primaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));
        m_activePrimaryCommandBuffers++;

        return *m_primaryCommandBuffers.back();
    case VK_COMMAND_BUFFER_LEVEL_SECONDARY:
        if( m_activeSecondaryCommandBuffers < m_secondaryCommandBuffers.size() )
        {
            return *m_secondaryCommandBuffers[m_activeSecondaryCommandBuffers++];
        }

        m_secondaryCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(*this, level));
        m_activeSecondaryCommandBuffers++;

        return *m_secondaryCommandBuffers.back();
    default:
        QUITFMT("Failed to request command buffer as level was not {} recognised.", (size_t)level);
    }
}

VkResult CommandPool::reset_pool()
{
    switch( m_resetMode )
    {
    case CommandBuffer::ResetMode::ResetIndividual:
        return reset_command_buffers();
    case CommandBuffer::ResetMode::AlwaysAllocate:
        m_primaryCommandBuffers.clear();
        m_activePrimaryCommandBuffers = 0;

        m_secondaryCommandBuffers.clear();
        m_activeSecondaryCommandBuffers = 0;

        return VK_SUCCESS;
    case CommandBuffer::ResetMode::ResetPool:
        return vkResetCommandPool(get_device().get_handle(), get_handle(), 0);
    default:
        QUITFMT("CommandBuffer ResetMode enum has not been implemented yet.");
    }
}

uint32_t CommandPool::get_family_index() const
{
    return m_queueFamilyIndex;
}

RenderFrame* CommandPool::get_render_frame() const
{
    return m_renderFrame;
}

size_t CommandPool::get_thread_index() const
{
    return m_threadIndex;
}

CommandBuffer::ResetMode CommandPool::get_reset_mode() const
{
    return m_resetMode;
}

VkResult CommandPool::reset_command_buffers()
{
    VkResult result = VK_SUCCESS;

    for( auto& buffer : m_primaryCommandBuffers )
    {
        result = buffer->reset(m_resetMode);
        if( result != VK_SUCCESS )
        {
            return result;
        }
    }
    m_activePrimaryCommandBuffers = 0;

    for( auto& buffer : m_secondaryCommandBuffers )
    {
        result = buffer->reset(m_resetMode);
        if( result != VK_SUCCESS )
        {
            return result;
        }
    }
    m_activeSecondaryCommandBuffers = 0;

    return result;
}

} // vk