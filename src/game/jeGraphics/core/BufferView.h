#pragma once

#include "vkcommon.h"

namespace vk
{

class Buffer;

class BufferView
{
public:
    BufferView() = default;
    BufferView(Buffer* buffer);
    BufferView(Buffer* buffer, VkDeviceSize offset, VkDeviceSize size);
    ~BufferView() = default;

    BufferView(BufferView&&) = default;
    BufferView(const BufferView&) = default;
    BufferView& operator=(BufferView&&) = default;
    BufferView& operator=(const BufferView&) = default;

    VkDeviceSize get_offset() const;
    VkDeviceSize get_size() const;

    const Buffer& get_buffer() const;

    VkDescriptorBufferInfo get_descriptor_info() const;

    u8* map();
private:
    Buffer* m_buffer;
    VkDeviceSize m_offset;
    VkDeviceSize m_size;
};

} // vk