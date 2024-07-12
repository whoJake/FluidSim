#include "Buffer.h"

#include "Device.h"

namespace vk
{

Buffer::Buffer(Device&            device,
               VkDeviceSize       size,
               VkBufferUsageFlags usage,
               VmaMemoryUsage     memoryUsage) :
    Resource(VK_NULL_HANDLE, device),
    m_size(std::max(size, device.get_gpu().get_properties().limits.minStorageBufferOffsetAlignment)),
    m_usage(usage),
    m_allocation(nullptr)
{
    VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    createInfo.size = m_size;
    createInfo.usage = usage;

    VmaAllocationCreateInfo allocInfo{ };
    allocInfo.flags = memoryUsage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST
        ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        : 0;
    allocInfo.usage = memoryUsage;

    VkResult result = vmaCreateBuffer(get_device().get_allocator(), &createInfo, &allocInfo, &m_handle, &m_allocation, nullptr);
    VK_CHECK(result, "Failed to create Buffer.");
}

Buffer::~Buffer() {
    get_device().wait_idle();

    if( m_mapped)
    {
        unmap();
    }

    vmaDestroyBuffer(get_device().get_allocator(), m_handle, m_allocation);
}

VkDeviceSize Buffer::get_size() const {
    return m_size;
}

VkBufferUsageFlags Buffer::get_usage() const {
    return m_usage;
}

const VmaAllocation& Buffer::get_allocation() const {
    return m_allocation;
}

uint8_t* Buffer::map()
{
    if( m_mapped )
    {
        return m_mappedData;
    }

    if( m_usage & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )
    {
        QUITFMT("Attempted to map an image that can't be mapped.");
    }

    VkResult result = vmaMapMemory(get_device().get_allocator(), m_allocation, (void**)&m_mappedData);
    VK_CHECK(result, "Failed to map image memory.");

    m_mapped = true;
    return m_mappedData;
}

void Buffer::unmap()
{
    if( !m_mapped )
    {
        GRAPHICS_WARN("Trying to unmap memory that is not mapped.");
        return;
    }

    vmaUnmapMemory(get_device().get_allocator(), m_allocation);
    m_mapped = false;
    m_mappedData = nullptr;
}

} // vk