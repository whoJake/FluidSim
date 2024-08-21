#pragma once

#include "CommandBuffer.h"

namespace vk
{

class Device;
class RenderFrame;

class CommandPool : public Resource<VkCommandPool>
{
public:
    CommandPool(Device&                  device,
                uint32_t                 queueFamilyIndex,
                RenderFrame*             renderFrame       = nullptr,
                size_t                   threadIndex       = 0,
                CommandBuffer::ResetMode resetMode         = CommandBuffer::ResetMode::ResetPool);

    ~CommandPool();

    CommandPool(CommandPool&&);
    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(CommandPool&&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;

    CommandBuffer& request_command_buffer(VkCommandBufferLevel level);

    VkResult reset_pool();

    uint32_t get_family_index() const;

    RenderFrame* get_render_frame() const;

    size_t get_thread_index() const;

    CommandBuffer::ResetMode get_reset_mode() const;

private:
    VkResult reset_command_buffers();

private:
    uint32_t m_queueFamilyIndex;
    RenderFrame* m_renderFrame;
    size_t m_threadIndex;
    CommandBuffer::ResetMode m_resetMode;

    std::vector<std::unique_ptr<CommandBuffer>> m_primaryCommandBuffers;
    uint32_t m_activePrimaryCommandBuffers = 0;

    std::vector<std::unique_ptr<CommandBuffer>> m_secondaryCommandBuffers;
    uint32_t m_activeSecondaryCommandBuffers = 0;
};

} // vk