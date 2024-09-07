#pragma once

#include "rendering/RenderContext.h"
#include "core/CommandPool.h"
#include "data/pool.h"
#include "data/queue.h"
#include <bitset>

namespace vk
{
class Buffer;
} // vk

namespace fw
{
namespace gfx
{

class StreamHandle
{
private:
    StreamHandle() = default;
    ~StreamHandle() = default;

    friend class Streamer;
    void* m_stream{ nullptr };
    bool m_discard{ false };
};

class Streamer
{
public:
    Streamer(vk::RenderContext& context);
    ~Streamer();

    StreamHandle* request(std::unique_ptr<vk::Buffer>&& source, VkBufferUsageFlags usage);

    bool is_loaded(StreamHandle* request) const;
    std::unique_ptr<vk::Buffer> access(StreamHandle* request);

    void discard(StreamHandle* request);

    void resolve_requests();

    static constexpr u64 default_max_active_streams = 16;
    static constexpr u64 default_max_pending_requests = 16;
private:
    struct Stream
    {
        std::unique_ptr<vk::Buffer> source{ nullptr };
        std::unique_ptr<vk::Buffer> destination{ nullptr };
        VkFence fence{ VK_NULL_HANDLE };
    };
private:
    Stream* get_next_free();
    void free_stream(Stream* stream);

    void begin_stream(Stream* stream, std::unique_ptr<vk::Buffer>&& source, VkBufferUsageFlags usage);
    void reset_buffers();

    vk::RenderContext& m_context;

    mtl::pool<Stream> m_streams;
    mtl::queue_v<VkFence> m_fences;

    mtl::queue_v<StreamHandle*> m_pendingRequests;

    vk::CommandPool m_bufferPool;
    u64 m_activeCmdBuffers{ 0 };
    static constexpr u64 max_active_cmd_buffers = 5;
};

} // gfx
} // fw