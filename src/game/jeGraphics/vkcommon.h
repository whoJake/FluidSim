#pragma once

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

#define VK_CHECK(result, msg, ...) do{ if(result != VK_SUCCESS){ QUITFMT(msg, __VA_ARGS__); } }while(0)

#define VK_ASSERT(condition, msg, ...) do{ if(!condition){ QUITFMT(msg, __VA_ARGS__); } }while(0)

#define MAX_VK_FENCE_TIMEOUT = std::numeric_limits<size_t>::max();

namespace vk
{

struct LoadStoreInfo {
    VkAttachmentLoadOp loadOp;
    VkAttachmentStoreOp storeOp;
};

struct ImageMemoryBarrier
{
    VkImageLayout oldLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
    VkImageLayout newLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
    VkAccessFlags srcAccessMask{ 0 };
    VkAccessFlags dstAccessMask{ 0 };
    uint32_t srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };
    uint32_t dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED };
    VkPipelineStageFlags srcStageMask{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
    VkPipelineStageFlags dstStageMask{ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
};

inline bool is_depth_only_format(VkFormat format) {
    return format == VK_FORMAT_D16_UNORM ||
        format == VK_FORMAT_D32_SFLOAT;
}

inline bool is_depth_stencil_format(VkFormat format) {
    return format == VK_FORMAT_D16_UNORM_S8_UINT ||
        format == VK_FORMAT_D24_UNORM_S8_UINT ||
        format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

inline bool is_depth_format(VkFormat format) {
    return is_depth_only_format(format) || is_depth_stencil_format(format);
}

inline VkFormat get_suitable_depth_format(VkPhysicalDevice             gpu,
                                          bool                         depthOnly          = false,
                                          const std::vector<VkFormat>& formatPriorityList = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM })
{
    VkFormat outputFormat{ VK_FORMAT_UNDEFINED };

    for( const VkFormat& priorityFormat : formatPriorityList )
    {
        // Check if we're a depth only format
        if( depthOnly && !is_depth_only_format(priorityFormat) )
        {
            continue;
        }

        VkFormatProperties properties{ };
        vkGetPhysicalDeviceFormatProperties(gpu, priorityFormat, &properties);

        // If we want stencil, we need this property for optimal tiling
        if( properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
        {
            outputFormat = priorityFormat;
            break;
        }
    }

    if( outputFormat != VK_FORMAT_UNDEFINED )
    {
        return outputFormat;
    }

    // Display formats supported/requested in this exception?
    QUITFMT("Failed to select a suitable depth format.");
    return VK_FORMAT_UNDEFINED;
}

template <class T>
inline uint32_t to_u32(T val) {
    return static_cast<uint32_t>(val);
}

#define BYTE_TO_BINARY_STREAM(byte)  \
    ((byte) & 0x80 ? "1" : "0") << \
    ((byte) & 0x40 ? "1" : "0") << \
    ((byte) & 0x20 ? "1" : "0") << \
    ((byte) & 0x10 ? "1" : "0") << \
    ((byte) & 0x08 ? "1" : "0") << \
    ((byte) & 0x04 ? "1" : "0") << \
    ((byte) & 0x02 ? "1" : "0") << \
    ((byte) & 0x01 ? "1" : "0") 

template <typename T>
inline std::string to_bit_string(T val) {
    std::stringstream ss;
    ss << "0b";
    for( long long i = sizeof(T) - 1; i >= 0; i-- ) {
        T shiftval = static_cast<T>(i * 8);
        ss << BYTE_TO_BINARY_STREAM(val >> shiftval);
        ss << " ";
    }
    return ss.str();
}

/**
* @brief Reinterpret casts a VkPhysicalDeviceFeatures struct into an array of 55 VkBool's
*/
inline std::array<VkBool32, 55> to_boolarr(VkPhysicalDeviceFeatures features) {
    std::array<VkBool32, 55> result{ };
    VkBool32* arrptr = reinterpret_cast<VkBool32*>(&features);
    std::move(arrptr, arrptr + 55, result.begin());
    return result;
}

inline VkPhysicalDeviceFeatures operator_bitor(const VkPhysicalDeviceFeatures& first, const VkPhysicalDeviceFeatures& second) {
    std::array<VkBool32, 55> one = to_boolarr(first);
    std::array<VkBool32, 55> two = to_boolarr(second);

    std::array<VkBool32, 55> result{ };
    for( size_t i = 0; i < one.size(); i++ ) {
        result[i] = one[i] | two[i];
    }

    /// Cast back to VkPhysicalDeviceFeatures
    return *reinterpret_cast<VkPhysicalDeviceFeatures*>(&result[0]);
}

/**
* @brief Find the features that are either supported or unsupported between two feature sets
*
* @param "requested" The feature set that has been requested
* @param "available" The feature set that is available
* @param "supported" (Optional) Whether or not supported or unsupported features should be returned
*
* @returns Vector of the string names of supported or unsupported features
*/
static std::vector<std::string> get_device_features_overlap(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& available, bool supported = true) {
    const std::string nameLookup[55]{
        "robustBufferAccess", "fullDrawIndexUint32", "imageCubeArray", "independentBlend", "geometryShader",
        "tessellationShader", "sampleRateShading", "dualSrcBlend", "logicOp", "multiDrawIndirect",
        "drawIndirectFirstInstance", "depthClamp", "depthBiasClamp", "fillModeNonSolid", "depthBounds",
        "wideLines", "largePoints", "alphaToOne", "multiViewport", "samplerAnisotropy", "textureCompressionETC2",
        "textureCompressionASTC_LDR", "textureCompressionBC", "occlusionQueryPrecise", "pipelineStatisticsQuery",
        "vertexPipelineStoresAndAtomics", "fragmentStoresAndAtomics", "shaderTessellationAndGeometryPointSize",
        "shaderImageGatherExtended", "shaderStorageImageExtendedFormats", "shaderStorageImageMultisample",
        "shaderStorageImageReadWithoutFormat", "shaderStorageImageWriteWithoutFormat", "shaderUniformBufferArrayDynamicIndexing",
        "shaderSampledImageArrayDynamicIndexing", "shaderStorageBufferArrayDynamicIndexing",
        "shaderStorageImageArrayDynamicIndexing", "shaderClipDistance", "shaderCullDistance", "shaderFloat64",
        "shaderInt64", "shaderInt16", "shaderResourceResidency", "shaderResourceMinLod", "sparseBinding",
        "sparseResidencyBuffer", "sparseResidencyImage2D", "sparseResidencyImage3D", "sparseResidency2Samples",
        "sparseResidency4Samples", "sparseResidency8Samples", "sparseResidency16Samples", "sparseResidencyAliased",
        "variableMultisampleRate", "inheritedQueries"
    };
    std::array<VkBool32, 55> req = to_boolarr(requested);
    std::array<VkBool32, 55> avail = to_boolarr(available);
    std::vector<std::string> result{ };

    for( size_t i = 0; i < req.size(); i++ ) {
        if( supported ) {
            if( req[i] && avail[i] )
                result.push_back(nameLookup[i]);
        }
        else {
            if( req[i] && !avail[i] )
                result.push_back(nameLookup[i]);
        }
    }

    return result;
}

//--------------------
//------CACHING-------
//--------------------

/*
* IF THERE ARE COMPILE ERRORS EG.
* CG2660 'std::pair<a, b>::pair': function does not take 2 arguments.
* THEN ITS LIKELY A MISSING COPY CONSTRUCTOR ON ONE OF THE RESOURCES ( Resource::Resource(Resource&&) )
*/

template<typename T, typename... A>
inline T& request_resource(const jclog::Log& log, std::unordered_map<size_t, T>& resources, A&... args)
{
    size_t hash{ 0 };
    hash_params(hash, args...);

    auto it = resources.find(hash);
    if( it != resources.end() )
        return (*it).second;

    // Could be reduced to a single .emplace call but its useful to know when an object is being created before creating it

    JCLOG_TRACK(log, "Generating #{} ( {} ) cache object", resources.size(), typeid(T).name());
    T resource(args...);
    auto ins_it = resources.emplace(hash, std::move(resource));
    return (ins_it.first)->second;
}

template<typename T, typename... A>
inline T& request_resource(std::unordered_map<size_t, T>& resources, A&... args)
{
    return request_resource(jclog::Log(), resources, args...);
}

//---------------------
//-------HASHING-------
//---------------------

template<class T>
inline void hash_combine(size_t& seed, const T& b)
{
    std::hash<T> hasher;
    size_t hash = hasher(b);

    // Taken from glm::detail::hash_combine
    hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash;
}

template<typename T, typename... A>
inline void hash_params(size_t& seed, const T& param)
{
    hash_combine(seed, param);
}

template<typename T, typename... A>
inline void hash_params(size_t& seed, const T& firstArg, const A&... args)
{
    hash_combine(seed, firstArg);
    hash_params(seed, args...);
}

class Device;
class ShaderModule;
struct ShaderResource;
class RenderPass;
class RenderTarget;

} // vk

namespace std
{

template<typename T>
struct hash<vector<T>>
{
    size_t operator()(const vector<T>& tVec) const
    {
        size_t result = 0;
        for( auto& t : tVec )
        {
            vk::hash_combine(result, t);
        }

        return result;
    }
};

template<>
struct hash<VkExtent2D>
{
    size_t operator()(const VkExtent2D& extent) const
    {
        size_t result = 0;
        vk::hash_combine(result, extent.width);
        vk::hash_combine(result, extent.height);
        return result;
    }
};

template<>
struct hash<VkExtent3D>
{
    size_t operator()(const VkExtent3D& extent) const
    {
        size_t result = 0;
        vk::hash_combine(result, VkExtent2D{ extent.width, extent.height });
        vk::hash_combine(result, extent.depth);
        return result;
    }
};

template<>
struct hash<VkDescriptorType>
{
    size_t operator()(const VkDescriptorType& type) const
    {
        std::hash<std::underlying_type<VkDescriptorType>::type> hasher;
        return hasher(type);
    }
};

template<>
struct hash<vk::Device>
{
    size_t operator()(const vk::Device& device) const;
};

template<>
struct hash<vk::ShaderModule>
{
    size_t operator()(const vk::ShaderModule& shaderModule) const;
};

template<>
struct hash<VkShaderStageFlagBits>
{
    size_t operator()(const VkShaderStageFlagBits& bits) const
    {
        std::hash<uint32_t> hasher;
        return hasher(static_cast<uint32_t>(bits));
    }
};

template<>
struct hash<vector<uint8_t>>
{
    size_t operator()(const vector<uint8_t>& vec) const
    {
        std::hash<string> strhash;
        return strhash(std::string(vec.begin(), vec.end()));
    }
};

template<>
struct hash<VkDescriptorBufferInfo>
{
    size_t operator()(const VkDescriptorBufferInfo& info) const
    {
        size_t result;
        vk::hash_combine(result, (void*)info.buffer);
        vk::hash_combine(result, info.offset);
        vk::hash_combine(result, info.range);
        return result;
    }
};

template<>
struct hash<VkDescriptorImageInfo>
{
    size_t operator()(const VkDescriptorImageInfo& info) const
    {
        size_t result;
        vk::hash_combine(result, (void*)info.imageLayout);
        vk::hash_combine(result, (void*)info.imageView);
        vk::hash_combine(result, (void*)info.sampler);
        return result;
    }
};

template<>
struct hash<vk::ShaderResource>
{
    size_t operator()(const vk::ShaderResource& resource) const;
};

template<>
struct hash<vk::RenderPass>
{
    size_t operator()(const vk::RenderPass& pass) const;
};

template<>
struct hash<vk::RenderTarget>
{
    size_t operator()(const vk::RenderTarget& target) const;
};

} // std