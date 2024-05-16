#pragma once

#include "vkcommon.h"
#include "Resource.h"
#include "PipelineState.h"
#include "ImageView.h"

namespace vk
{

class Device;
class CommandPool;
class RenderTarget;
class RenderPass;
class Framebuffer;
class PipelineLayout;
class DescriptorSetLayout;
class Pipeline;
class Buffer;

class CommandBuffer : public Resource<VkCommandBuffer>
{
public:
    enum class ResetMode
    {
        ResetPool,
        ResetIndividual,
        AlwaysAllocate,
    };

    struct RenderPassBinding
    {
        const RenderPass* renderPass{ VK_NULL_HANDLE};
        const Framebuffer* framebuffer{ VK_NULL_HANDLE};
    };

    CommandBuffer(CommandPool& commandPool, VkCommandBufferLevel level);
    ~CommandBuffer();

    VkResult reset(ResetMode resetMode);

    VkCommandBufferLevel get_level() const;

    VkResult begin(VkCommandBufferUsageFlags usage, const RenderPass* renderPass, const Framebuffer* framebuffer, uint32_t subpassIndex);

    VkResult end();

    void begin_render_pass(const RenderTarget*              renderTarget,
                           const RenderPass&                renderPass,
                           const Framebuffer&               framebuffer,
                           const std::vector<VkClearValue>& clearValues,
                           VkSubpassContents                contents = VK_SUBPASS_CONTENTS_INLINE);

    void end_render_pass();

    void bind_pipeline_layout(PipelineLayout& layout);

    void bind_pipeline(Pipeline& pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    void set_viewport(VkViewport viewport);

    void set_scissor(VkRect2D scissor);
    
    void push_constants(PipelineLayout& layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pData);

    void bind_vertex_buffers(Buffer& buffer, uint32_t binding);

    void bind_vertex_buffers(const std::vector<Buffer*>& buffers, uint32_t firstBinding);

    void bind_index_buffer(Buffer& buffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32);

    void draw_indexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);

    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);

    void image_pipeline_barrier(const ImageView&   imageView,
                                ImageMemoryBarrier memoryBarrier);

    inline PipelineState& get_pipeline_state() { return m_state; }
private:
    CommandPool& m_commandPool;
    VkCommandBufferLevel m_level;

    RenderPassBinding m_currentRenderPass{ };
    PipelineState m_state{ };
    std::unordered_map<uint32_t, DescriptorSetLayout*> m_descriptorSetLayoutBindingState;
};

} // vk