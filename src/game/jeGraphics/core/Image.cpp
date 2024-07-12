#include "Image.h"
#include "ImageView.h"
#include "Device.h"

namespace vk
{

inline static VkImageType get_image_type(VkExtent3D extent)
{
    VkImageType result{ };
    char dimension{ 0 };
    if( extent.width > 1 )
        dimension++;
    if( extent.height > 1 )
        dimension++;
    if( extent.depth > 1 )
        dimension++;

    switch( dimension )
    {
    case 1:
        result = VK_IMAGE_TYPE_1D;
        break;
    case 2:
        result = VK_IMAGE_TYPE_2D;
        break;
    case 3:
        result = VK_IMAGE_TYPE_3D;
        break;
    default:
        QUITFMT("Failed to find requested image type for extent: {}, {}, {}", extent.width, extent.height, extent.depth);
    }

    return result;
}

Image::Image(Device&               device,
             const VkExtent3D&     extent,
             VkFormat              format,
             VkImageUsageFlags     usage,
             VmaMemoryUsage        memoryUsage,
             VkSampleCountFlagBits sampleCount,
             uint32_t              mipLevels,
             uint32_t              arrayLayers,
             VkImageTiling         tiling,
             VkImageCreateFlags    flags,
             uint32_t              numQueueFamilies,
             const uint32_t*       pQueueFamilies) :
    Resource(VK_NULL_HANDLE, device),
    m_allocation(nullptr),
    m_type(get_image_type(extent)),
    m_extent(extent),
    m_format(format),
    m_usage(usage),
    m_sampleCount(sampleCount),
    m_tiling(tiling),
    m_arrayLayerCount(arrayLayers),
    m_mappedData(nullptr)
{
    if( mipLevels == 0 )
        QUITFMT("Image needs at least one mip level.");

    if( arrayLayers == 0 )
        QUITFMT("Image needs atleast one array layer.");

    m_subresource.mipLevel = mipLevels;
    m_subresource.arrayLayer = arrayLayers;

    VkImageCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    createInfo.imageType = m_type;
    createInfo.extent = extent;
    createInfo.format = format;
    createInfo.usage = usage;
    createInfo.samples = sampleCount;
    createInfo.mipLevels = mipLevels;
    createInfo.arrayLayers = arrayLayers;
    createInfo.tiling = tiling;
    createInfo.flags = flags;

    if( numQueueFamilies > 0 )
    {
        createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = numQueueFamilies;
        createInfo.pQueueFamilyIndices = pQueueFamilies;
    }
    else
    {
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VmaAllocationCreateInfo allocInfo{ };
    allocInfo.usage = memoryUsage;
    allocInfo.flags = memoryUsage == VMA_MEMORY_USAGE_AUTO_PREFER_HOST
        ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        : 0;

    VkResult result = vmaCreateImage(get_device().get_allocator(), &createInfo, &allocInfo, &m_handle, &m_allocation, nullptr);
    VK_CHECK(result, "Failed to create Image.");
}

Image::Image(Device&               device,
             VkImage               handle,
             const VkExtent3D&     extent,
             VkFormat              format,
             VkImageUsageFlags     usage,
             VkSampleCountFlagBits sampleCount) :
    Resource(handle, device),
    m_type(get_image_type(extent)),
    m_extent(extent),
    m_format(format),
    m_usage(usage),
    m_sampleCount(sampleCount),
    m_mappedData(nullptr)
{
    m_subresource.mipLevel = 1;
    m_subresource.arrayLayer = 1;
}

Image::~Image()
{
    if( m_mapped )
    {
        GRAPHICS_WARN("Destroying an Image which is currently still mapped.");
        unmap();
    }

    if( m_allocation )
    {
        vmaDestroyImage(get_device().get_allocator(), m_handle, m_allocation);
    }
}

Image::Image(Image&& other) :
    Resource(std::move(other)),
    m_allocation(other.m_allocation),
    m_type(other.m_type),
    m_extent(other.m_extent),
    m_format(other.m_format),
    m_usage(other.m_usage),
    m_sampleCount(other.m_sampleCount),
    m_tiling(other.m_tiling),
    m_subresource(other.m_subresource),
    m_arrayLayerCount(other.m_arrayLayerCount),
    m_views(std::exchange(other.m_views, { })),
    m_mappedData(other.m_mappedData),
    m_mapped(other.m_mapped)
{
    other.m_allocation = nullptr;
    other.m_mappedData = nullptr;
    other.m_mapped = false;
    other.m_views.clear();

    // Remove dangling pointers
    for( auto& view : m_views )
        view->set_image(*this);
}

const VmaAllocation& Image::get_allocation() const
{
    return m_allocation;
}

uint8_t* Image::map()
{
    if( m_mapped )
        return m_mappedData;

    if( m_tiling != VK_IMAGE_TILING_LINEAR )
        GRAPHICS_WARN("Mapping image memory that is not linear.");

    VkResult result = vmaMapMemory(get_device().get_allocator(), m_allocation, (void**)&m_mappedData);
    VK_CHECK(result, "Failed to map image memory.");

    m_mapped = true;

    return m_mappedData;
}

void Image::unmap()
{
    if( !m_mapped )
    {
        GRAPHICS_WARN("Trying to unmap memory that is not mapped.");
        return;
    }

    vmaUnmapMemory(get_device().get_allocator(), m_allocation);
    m_mapped = false;
}

VkImageType Image::get_type() const
{
    return m_type;
}

const VkExtent3D& Image::get_extent() const
{
    return m_extent;
}

VkFormat Image::get_format() const
{
    return m_format;
}

VkSampleCountFlagBits Image::get_sample_count() const
{
    return m_sampleCount;
}

VkImageUsageFlags Image::get_usage() const
{
    return m_usage;
}

VkImageTiling Image::get_tiling() const
{
    return m_tiling;
}

VkImageSubresource Image::get_subresource() const
{
    return m_subresource;
}

uint32_t Image::get_array_layer_count() const
{
    return m_arrayLayerCount;
}

std::unordered_set<ImageView*>& Image::get_views()
{
    return m_views;
}

} // vk