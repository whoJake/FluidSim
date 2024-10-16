#include "vma_allocator.h"

namespace gfx
{

u32 vma_allocator::initialise(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    VmaAllocatorCreateInfo createInfo{ };
    createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    createInfo.physicalDevice = physicalDevice;
    createInfo.device = device;
    createInfo.instance = instance;

    VkResult result = vmaCreateAllocator(&createInfo, &m_handle);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to create VMA allocator.");
    }
}

void vma_allocator::shutdown()
{
    vmaDestroyAllocator(m_handle);
}

u8* vma_allocator::map(VmaAllocation allocation)
{
    void* data;
    VkResult result = vmaMapMemory(m_handle, allocation, &data);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Failed to allocate memory for mapping.");
    }

    return static_cast<u8*>(data);
}

void vma_allocator::unmap(VmaAllocation allocation)
{
    vmaUnmapMemory(m_handle, allocation);
}

vma_allocation<VkBuffer> vma_allocator::allocate_buffer(u64 size, buffer_usage usage, memory_type mem_type)
{
    vma_allocation<VkBuffer> retval{ };

    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = static_cast<VkBufferUsageFlags>(usage);

    VmaAllocationCreateInfo allocInfo{ };
    switch( mem_type )
    {
    case memory_type::cpu_accessible:
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        break;
    case memory_type::gpu_only:
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        break;
    }

    VkResult result = vmaCreateBuffer(m_handle, &bufferInfo, &allocInfo, &retval.resource, &retval.allocation, nullptr);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Allocation failed.");
    }

    return retval;
}

void vma_allocator::free_buffer(vma_allocation<VkBuffer> allocation)
{
    vmaDestroyBuffer(m_handle, allocation.resource, allocation.allocation);
}

} // gfx