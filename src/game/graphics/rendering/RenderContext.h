#pragma once

#include "vkcommon.h"
#include "core/Swapchain.h"
#include "RenderFrame.h"
#include "core/Buffer.h"
#include "RenderTarget.h"

namespace vk
{

class Device;

class RenderContext
{
public:
    RenderContext(Device&                                device,
                  VkSurfaceKHR                           surface,
                  VkExtent2D                             surfaceExtent,
                  const std::vector<VkPresentModeKHR>&   presentModePriorityList,
                  const std::vector<VkSurfaceFormatKHR>& surfaceFormatPriorityList,
                  RenderTarget::CreateFunc               renderTargetCreateFunc = RenderTarget::default_create_function,
                  size_t                                 threadCount = 1);

    ~RenderContext() = default;
    
    RenderContext(RenderContext&&);

    VkSemaphore submit(const Queue&                             queue,
                       CommandBuffer&                           commandBuffer,
                       const std::vector<VkSemaphore>&          waitSemaphores   = { },
                       const std::vector<VkPipelineStageFlags>& waitStageFlags   = { });

    VkSemaphore submit(const Queue&                             queue,
                       const std::vector<CommandBuffer*>&       commandBuffers,
                       const std::vector<VkSemaphore>&          waitSemaphores   = { },
                       const std::vector<VkPipelineStageFlags>& waitStageFlags   = { });

    /// <summary>
    /// Begin a command buffer on the current frame, starting the frame if it hasn't been already.
    /// </summary>
    /// <param name="resetmode"></param>
    /// <returns>Fresh command buffer to record to.</returns>
    CommandBuffer& begin(CommandBuffer::ResetMode resetmode = CommandBuffer::ResetMode::ResetPool);

    void begin_frame();

    void end_frame(VkSemaphore semaphore);

    void submit_and_end(CommandBuffer& commandBuffer);

    void submit_and_end(const std::vector<CommandBuffer*>& commandBuffers);

    void wait_frame();

    RenderFrame& get_active_frame();

    uint32_t get_frame_index() const;

    uint32_t get_active_render_frame_index() const;

    bool handle_surface_changes(bool forceUpdate = false);

    void update_swapchain(VkExtent2D extent);

    inline VkSemaphore get_aquired_semaphore() const { return m_aquiredSemaphore; }

    inline const SwapchainProperties& get_swapchain_properties() const { return m_swapchainProperties; }

    void recreate();

    Device& get_device();
private:
    Device& m_device;

    VkExtent2D m_surfaceExtent;
    size_t m_threadCount;
    RenderTarget::CreateFunc m_renderTargetCreateFunc;

    std::unique_ptr<Swapchain> m_swapchain;
    SwapchainProperties m_swapchainProperties;

    // Denotes that we have aquired the next swapchain image.
    VkSemaphore m_aquiredSemaphore;

    std::vector<std::unique_ptr<RenderFrame>> m_frames;
    uint32_t m_frameIndex{ 0 };
    uint32_t m_activeRenderingFrameIndex{ 0 };
    bool m_activeRenderFrame{ false };
};

} // vk