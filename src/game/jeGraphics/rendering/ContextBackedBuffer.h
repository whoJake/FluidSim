#pragma once

#include "RenderContext.h"
#include "core/Buffer.h"

namespace vk
{

class ContextBackedBuffer
{
public:
    ContextBackedBuffer(RenderContext&      context,
                        VkDeviceSize        size,
                        VkBufferUsageFlags  usage,
                        VmaMemoryUsage      memoryUsage);
    ~ContextBackedBuffer() = default;

    void create_new_buffer(VkDeviceSize size);

    bool carry_over_buffer();

    vk::Buffer* get_buffer(uint32_t frameIndex) const;

    VkDeviceSize get_size() const;

    VkBufferUsageFlags get_usage() const;

    const VmaAllocation& get_allocation() ;

    uint8_t* map();

    void unmap();
private:
    vk::Buffer* get_buffer(uint32_t index);

    void create_buffer(uint32_t index);
private:
    RenderContext* m_context;
    std::vector<std::shared_ptr<vk::Buffer>> m_buffers;
    std::vector<bool> m_validity; // <-- use validity

    VkDeviceSize m_size;
    VkBufferUsageFlags m_usage;
    VmaMemoryUsage m_memoryUsage;
};

} // vk