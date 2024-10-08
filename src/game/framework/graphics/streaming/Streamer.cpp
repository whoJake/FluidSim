#include "Streamer.h"
#include "core/Device.h"

namespace fw
{
namespace gfx
{

MAKEPARAM(gfx_streaming_max_active_streams);
MAKEPARAM(gfx_streaming_max_pending_requests);

static u64 get_max_active_streams()
{
    if( p_gfx_streaming_max_active_streams.get() )
    {
        return p_gfx_streaming_max_active_streams.as_u64();
    }

    return Streamer::default_max_active_streams;
}

static u64 get_max_pending_requests()
{
    if( p_gfx_streaming_max_pending_requests.get() )
    {
        return p_gfx_streaming_max_pending_requests.as_u64();
    }

    return Streamer::default_max_pending_requests;
}

Streamer::Streamer(vk::RenderContext& context) :
    m_context(context),
    m_streams(u32_cast(get_max_active_streams())),
    m_fences(get_max_active_streams()),
    m_pendingRequests(get_max_pending_requests()),
    m_bufferPool(m_context.get_device(),
                 m_context.get_device().get_queue_by_flags(VK_QUEUE_TRANSFER_BIT, 0).get_family_index(),
                 nullptr,
                 0,
                 vk::CommandBuffer::ResetMode::ResetPool)
{
    // Initialise our fences, unsignalled. We will reset fences when a stream is accessed and the
    // request is destroyed.
    VkFenceCreateInfo info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

    for( u32 idx = 0; idx < get_max_active_streams(); idx++ )
    {
        VkFence fence;
        vkCreateFence(m_context.get_device().get_handle(), &info, nullptr, &fence);
        m_fences.push_back(fence);
    }
}

Streamer::~Streamer()
{
    while( !m_pendingRequests.empty() )
    {
        StreamHandle* request;
        m_pendingRequests.pop_front(&request);
        if( request->m_discard != true )
        {
            SYSLOG_ERROR("Stream request not properly disposed of.");
        }

        delete request;
    }

    std::vector<VkFence> activeFences(m_streams.size());
    m_streams.for_each([&](Stream* stream)
        {
            activeFences.push_back(stream->fence);
        });

    if( !activeFences.empty() )
    {
        // maybe dont endlessly wait for fences in destructor?
        vkWaitForFences(m_context.get_device().get_handle(), u32_cast(activeFences.size()), activeFences.data(), VK_TRUE, u64_max);
    }

    // Fences can only be destroyed once operations that rely on them have completed, hence wiating.
    for( VkFence fence : activeFences )
    {
        vkDestroyFence(m_context.get_device().get_handle(), fence, nullptr);
    }

    VkFence fence;
    while( m_fences.pop_front(&fence) )
    {
        vkDestroyFence(m_context.get_device().get_handle(), fence, nullptr);
    }
}

StreamHandle* Streamer::request(std::unique_ptr<vk::Buffer>&& source, VkBufferUsageFlags usage)
{
    StreamHandle* request = new StreamHandle();

    Stream* next_slot = get_next_free();
    if( next_slot == nullptr )
    {
        m_pendingRequests.push_back(request);
        return request;
    }

    request->m_stream = reinterpret_cast<void*>(next_slot);
    begin_stream(next_slot, std::move(source), usage);
    return request;
}

bool Streamer::is_loaded(StreamHandle* request) const
{
    if( request->m_stream == nullptr )
    {
        return false;
    }

    Stream* stream = reinterpret_cast<Stream*>(request->m_stream);
    return vkGetFenceStatus(m_context.get_device().get_handle(), stream->fence) == VK_SUCCESS;
}

std::unique_ptr<vk::Buffer> Streamer::access(StreamHandle* request)
{
    if( !is_loaded(request) )
    {
        return nullptr;
    }

    Stream* stream = reinterpret_cast<Stream*>(request->m_stream);

    // return destination buffer
    // Keep this locally so we can free this stream as soon as possible.
    std::unique_ptr<vk::Buffer> retval = std::move(stream->destination);

    free_stream(reinterpret_cast<Stream*>(request->m_stream));
    delete request;


    return std::move(retval);
}

void Streamer::discard(StreamHandle* request)
{
    request->m_discard = true;
}

void Streamer::resolve_requests()
{
    // UNFINISHED
    return;

    const u32 max_request_per_resolve = 5;
    u32 resolveCount = 0;
    while( !m_pendingRequests.empty() && resolveCount < max_request_per_resolve )
    {
        Stream* nextStream = get_next_free();
        if( nextStream == nullptr )
        {
            // pool is full.
            return;
        }

        StreamHandle* request;
        m_pendingRequests.pop_front(&request);

        return;
    }
    return;
}

Streamer::Stream* Streamer::get_next_free()
{
    return m_streams.allocate();
}

void Streamer::free_stream(Stream* stream)
{
    u64 freedStreamSize = stream->source->get_size();
    stream->source.reset();

    // Assume we've already retrieved the ptr so release just incase.
    stream->destination.release();
    vkResetFences(m_context.get_device().get_handle(), 1, &stream->fence);
    m_fences.push_back(std::move(stream->fence));
    m_streams.free(stream);

    CHANNEL_LOG_PROFILE(sys::log::channel::streaming, "Freed stream of size {}. Current streams: {}", freedStreamSize, m_streams.size());
}

void Streamer::begin_stream(Stream* stream, std::unique_ptr<vk::Buffer>&& source, VkBufferUsageFlags usage)
{
    // claim our fence.
    m_fences.pop_front(&stream->fence);

    CHANNEL_LOG_PROFILE(sys::log::channel::streaming, "Beginning stream of size {}. Current streams: {}", source->get_size(), m_streams.size());
    stream->source = std::move(source);

    stream->destination = std::make_unique<vk::Buffer>(
        m_context.get_device(),
        stream->source->get_size(),
        usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    );

    vk::CommandBuffer& command = m_bufferPool.request_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    m_activeCmdBuffers++;

    command.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr, nullptr, 0);

    VkBufferCopy copy{ };
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = stream->source->get_size();

    vkCmdCopyBuffer(command.get_handle(), stream->source->get_handle(), stream->destination->get_handle(), 1, &copy);
    command.end();

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command.get_handle();

    const vk::Queue& queue = m_context.get_device().get_queue_by_flags(VK_QUEUE_TRANSFER_BIT, m_bufferPool.get_family_index());
    queue.submit({ submitInfo }, stream->fence);

    reset_buffers();
}

void Streamer::reset_buffers()
{
    if( m_activeCmdBuffers < max_active_cmd_buffers )
    {
        return;
    }

    m_bufferPool.reset_pool();
    m_activeCmdBuffers = 0;
}

} // gfx
} // fw