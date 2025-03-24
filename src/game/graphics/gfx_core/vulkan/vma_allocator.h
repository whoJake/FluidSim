#pragma once

#include "../gfxdefines.h"
#include "vkdefines.h"

#include "../buffer.h"
#include "../texture.h"

namespace gfx
{

template<typename res>
struct vma_allocation
{
    res resource;
    VmaAllocation allocation;
};

class vma_allocator
{
public:
    vma_allocator() = default;
    ~vma_allocator() = default;

    DELETE_COPY(vma_allocator);
    DELETE_MOVE(vma_allocator);

    u32 initialise(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    void shutdown();

    u8* map(VmaAllocation allocation);
    void unmap(VmaAllocation allocation);

    vma_allocation<VkBuffer> allocate_buffer(u64 size, buffer_usage usage, memory_type mem_type);
    void free_buffer(vma_allocation<VkBuffer> allocation);

    vma_allocation<VkImage> allocate_image(texture_info info, resource_view_type type, memory_type mem_type);
    void free_image(vma_allocation<VkImage> allocation);
private:
    VmaAllocator m_handle;
};

} // gfx