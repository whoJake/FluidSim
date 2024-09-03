#include "UniformBuffer.h"

#include "shader_parser/ShaderResource.h"
#include "core/Buffer.h"
#include "core/BufferView.h"

namespace fw
{
namespace gfx
{

static u64 get_aligned_size(vk::RenderContext& context, u64 size)
{
    u64 minAlignment = context.get_device().get_gpu().get_properties().limits.minUniformBufferOffsetAlignment;
    
    return static_cast<u64>(std::ceil(static_cast<f64>(size) / minAlignment)) * minAlignment;
}

UniformBuffer::UniformBuffer(vk::RenderContext& context,
                             u64 size,
                             UniformBufferType type,
                             u32 count,
                             VkBufferUsageFlagBits flags)
{
    m_size = size;
    m_alignedSize = get_aligned_size(context, m_size);
    m_count = count;

    switch( type )
    {
    case UniformBufferType::Dynamic:
        m_frameBlocks = context.get_swapchain_properties().imageCount;
        break;
    case UniformBufferType::Static:
        m_frameBlocks = 1;
        break;
    }

    m_buffer = std::make_unique<vk::Buffer>(
        context.get_device(),
        m_alignedSize * m_count * m_frameBlocks,
        flags,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST
    );
}

UniformBuffer::UniformBuffer(vk::RenderContext& context,
                             vk::ShaderResource* resource,
                             UniformBufferType type,
                             u32 count,
                             VkBufferUsageFlagBits flags) :
    UniformBuffer(context, resource->stride, type, count, flags)
{ }

u8* UniformBuffer::map(u32 frameIdx, u32 idx)
{
    return m_buffer->map() + get_offset(frameIdx, idx);
}

void UniformBuffer::unmap()
{
    m_buffer->unmap();
}

u64 UniformBuffer::get_size() const
{
    return m_size;
}

vk::BufferView UniformBuffer::get_buffer_view(u32 frameIdx, u32 idx) const
{
    return vk::BufferView(
        m_buffer.get(),
        get_offset(frameIdx, idx),
        m_size
    );
}

vk::Buffer& UniformBuffer::get_buffer()
{
    return *m_buffer;
}

u64 UniformBuffer::get_frame_block_size() const
{
    return u64_cast(m_count) * m_alignedSize;
}

u64 UniformBuffer::get_offset(u32 frameIdx, u32 idx) const
{
    return (get_frame_block_size() * frameIdx) + (m_alignedSize * idx);
}

} // gfx
} // fw