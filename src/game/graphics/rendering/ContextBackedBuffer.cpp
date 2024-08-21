#include "ContextBackedBuffer.h"

namespace vk
{

ContextBackedBuffer::ContextBackedBuffer(RenderContext&      context,
                                         VkDeviceSize        size,
                                         VkBufferUsageFlags  usage,
                                         VmaMemoryUsage      memoryUsage) :
    m_context(&context),
    m_validity(context.get_swapchain_properties().imageCount, false),
    m_size(size),
    m_usage(usage),
    m_memoryUsage(memoryUsage)
{
    m_buffers.resize(m_context->get_swapchain_properties().imageCount);
}

void ContextBackedBuffer::create_new_buffer(VkDeviceSize size)
{
    m_size = size;
    uint32_t frameIndex = m_context->get_frame_index();
    vk::Buffer* currentBuffer = m_buffers.at(frameIndex).get();
    if( currentBuffer == nullptr )
    {
        create_buffer(frameIndex);
        return;
    }

    // Check if we can reuse the current buffer
    uint32_t previousFrameIndex = ( frameIndex - 1 ) % m_context->get_swapchain_properties().imageCount;
    vk::Buffer* previousBuffer = m_buffers.at(previousFrameIndex).get();
    if( m_validity.at(frameIndex)
        && previousBuffer != currentBuffer
        && currentBuffer->get_size() >= m_size )
    {
        // we can reuse this buffer
        return;
    }

    create_buffer(frameIndex);
}

bool ContextBackedBuffer::carry_over_buffer()
{
    uint32_t frameIndex = m_context->get_frame_index();
    uint32_t prevFrameIndex = ( frameIndex - 1 ) % m_context->get_swapchain_properties().imageCount;

    if( m_validity.at(frameIndex) )
    {
        return false;
    }

    if( m_validity.at(prevFrameIndex) )
    {
        m_buffers.at(frameIndex) = m_buffers.at(prevFrameIndex);
        m_validity.at(frameIndex) = true;
        return false;
    }

    create_new_buffer(m_size);
    return true;
}

vk::Buffer* ContextBackedBuffer::get_buffer(uint32_t frameIndex) const
{
    return m_buffers.at(frameIndex).get();
}

VkDeviceSize ContextBackedBuffer::get_size() const
{
    return m_size;
}

VkBufferUsageFlags ContextBackedBuffer::get_usage() const
{
    return m_usage;
}

const VmaAllocation& ContextBackedBuffer::get_allocation()
{
    return get_buffer(m_context->get_frame_index())->get_allocation();
}

uint8_t* ContextBackedBuffer::map()
{
    return get_buffer(m_context->get_frame_index())->map();
}

void ContextBackedBuffer::unmap()
{
    get_buffer(m_context->get_frame_index())->unmap();
}

vk::Buffer* ContextBackedBuffer::get_buffer(uint32_t index)
{
    vk::Buffer* retval = m_buffers.at(index).get();
    if( !retval )
    {
        uint32_t prevIndex = index % m_context->get_swapchain_properties().imageCount;
        retval = m_buffers.at(prevIndex).get();
        if( !retval )
        {
            create_buffer(index);
            retval = m_buffers.at(index).get();
        }
        else
        {
            m_buffers.at(index) = m_buffers.at(prevIndex);
        }
    }
    return retval;
}
void ContextBackedBuffer::create_buffer(uint32_t index)
{
    m_buffers.at(index) = std::make_shared<vk::Buffer>(
        m_context->get_device(),
        m_size,
        m_usage,
        m_memoryUsage);

    for( size_t i = 0; i < m_validity.size(); i++ )
    {
        if( i == index )
        {
            m_validity.at(i) = true;
            continue;
        }
        m_validity.at(i) = false;
    }
}

} // vk