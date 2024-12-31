#include "vma_allocator.h"
#include "vkconverts.h"

#ifdef GFX_SUPPORTS_VULKAN
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

    return 0;
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

vma_allocation<VkImage> vma_allocator::allocate_image(texture_info info, resource_view_type type, memory_type mem_type)
{
    vma_allocation<VkImage> retval{ };

    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.extent = { info.get_width(), info.get_height(), info.get_depth() };
    imageInfo.usage = static_cast<VkImageUsageFlags>(info.get_usage());
    imageInfo.format = converters::get_format_vk(info.get_format());

    // TODO
    imageInfo.mipLevels = 1;
    imageInfo.flags = 0;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices = 0;
    imageInfo.arrayLayers = 1;

    switch( type )
    {
    case resource_view_type::texture_1d:
        imageInfo.imageType = VK_IMAGE_TYPE_1D;
        break;
    case resource_view_type::texture_2d:
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        break;
    case resource_view_type::texture_3d:
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        break;
    case resource_view_type::cube:
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.depth = 0;
        imageInfo.arrayLayers = info.get_depth();
        break;
    }

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

    VkResult result = vmaCreateImage(m_handle, &imageInfo, &allocInfo, &retval.resource, &retval.allocation, nullptr);
    switch( result )
    {
    case VK_SUCCESS:
        break;
    default:
        GFX_ASSERT(false, "Allocation failed.");
    }

    return retval;
}

void vma_allocator::free_image(vma_allocation<VkImage> image)
{
    vmaDestroyImage(m_handle, image.resource, image.allocation);
}

} // gfx
#endif // GFX_SUPPORTS_VULKAN