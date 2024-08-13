#include "BufferView.h"
#include "Buffer.h"

namespace vk
{

BufferView::BufferView(Buffer* buffer) :
    m_buffer(buffer),
    m_offset(0),
    m_size(buffer->get_size())
{ }

BufferView::BufferView(Buffer* buffer, VkDeviceSize offset, VkDeviceSize size) :
    m_buffer(buffer),
    m_offset(offset),
    m_size(size)
{ }

VkDeviceSize BufferView::get_offset() const
{
    return m_offset;
}

VkDeviceSize BufferView::get_size() const
{
    return m_size;
}

const Buffer& BufferView::get_buffer() const
{
    return *m_buffer;
}

VkDescriptorBufferInfo BufferView::get_descriptor_info() const
{
    return
    {
        m_buffer->get_handle(),
        m_offset,
        m_size
    };
}

u8* BufferView::map()
{
    return m_buffer->map() + m_offset;
}

} // vk