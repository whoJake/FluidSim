#pragma once

#include "data/hash_string.h"
#include "rendering/RenderContext.h"

namespace vk
{
struct ShaderResource;
class Buffer;
class BufferView;
} // vk

namespace fw
{
namespace gfx
{

enum class UniformBufferType
{
    Dynamic,
    Static,
};

class UniformBuffer
{
public:
    UniformBuffer(vk::RenderContext& context,
                  u64 size,
                  UniformBufferType type,
                  u32 count = 1,
                  VkBufferUsageFlagBits flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    UniformBuffer(vk::RenderContext& context,
                  vk::ShaderResource* resource,
                  UniformBufferType type,
                  u32 count = 1,
                  VkBufferUsageFlagBits flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    ~UniformBuffer() = default;

    u8* map(u32 frameIdx = 0, u32 idx = 0);
    void unmap();

    u64 get_size() const;

    vk::BufferView get_buffer_view(u32 frameIdx = 0, u32 idx = 0) const;

    vk::Buffer& get_buffer();
private:
    u64 get_frame_block_size() const;
    u64 get_offset(u32 frameIdx, u32 idx) const;
private:
    std::unique_ptr<vk::Buffer> m_buffer;

    u64 m_size{ 0 };
    u64 m_alignedSize{ 0 };
    u32 m_count{ 0 };
    u32 m_frameBlocks{ 0 };
};

} // gfx
} // fw