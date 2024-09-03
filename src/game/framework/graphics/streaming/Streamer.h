#pragma once

#include "rendering/RenderContext.h"
#include "core/CommandPool.h"
#include "data/fixed_vector.h"
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
    i32 m_index{ invalid_index };

    static constexpr i32 invalid_index = -1;
    static constexpr i32 discard_index = -2;
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
    i32 get_next_free();
    void free_stream(i32 idx);

    void begin_stream(i32 idx, std::unique_ptr<vk::Buffer>&& source, VkBufferUsageFlags usage);
    void reset_buffers();

    vk::RenderContext& m_context;

    std::vector<bool> m_state;
    mtl::fixed_vector<Stream> m_streams;
    mtl::queue_v<StreamHandle*> m_pendingRequests;

    vk::CommandPool m_bufferPool;
    u64 m_activeCmdBuffers{ 0 };
    static constexpr u64 max_active_cmd_buffers = 5;
};

} // gfx
} // fw