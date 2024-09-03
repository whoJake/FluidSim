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
    m_state(get_max_active_streams()),
    m_streams(get_max_active_streams()),
    m_pendingRequests(get_max_pending_requests()),
    m_bufferPool(m_context.get_device(),
                 m_context.get_device().get_queue_by_flags(VK_QUEUE_TRANSFER_BIT, 0).get_family_index(),
                 nullptr,
                 0,
                 vk::CommandBuffer::ResetMode::ResetPool)
{
    // Initialise our fences, unsignalled. We will reset fences when a stream is accessed and the
    // request is destroyed.
    for( Stream& stream : m_streams )
    {
        VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        createInfo.flags = 0;
        
        vkCreateFence(m_context.get_device().get_handle(), &createInfo, nullptr, &stream.fence);
    }
}

Streamer::~Streamer()
{
    while( !m_pendingRequests.empty() )
    {
        StreamHandle* request;
        m_pendingRequests.pop_front(&request);
        if( request->m_index != StreamHandle::discard_index )
        {
            SYSLOG_ERROR("Stream request not properly disposed of.");
        }

        delete request;
    }

    std::vector<VkFence> allFences(m_streams.size());
    for( Stream& stream : m_streams )
    {
        allFences.push_back(stream.fence);
    }

    // maybe dont endlessly wait for fences in destructor?
    vkWaitForFences(m_context.get_device().get_handle(), u32_cast(allFences.size()), allFences.data(), VK_TRUE, u64_max);

    // Fences can only be destroyed once operations that rely on them have completed, hence wiating.
    for( VkFence fence : allFences )
    {
        vkDestroyFence(m_context.get_device().get_handle(), fence, nullptr);
    }
}

StreamHandle* Streamer::request(std::unique_ptr<vk::Buffer>&& source, VkBufferUsageFlags usage)
{
    StreamHandle* request = new StreamHandle();

    i32 next_slot = get_next_free();
    if( next_slot == StreamHandle::invalid_index )
    {
        m_pendingRequests.push_back(request);
        return request;
    }

    request->m_index = next_slot;
    begin_stream(request->m_index, std::move(source), usage);
    return request;
}

bool Streamer::is_loaded(StreamHandle* request) const
{
    if( request->m_index == StreamHandle::invalid_index )
    {
        return false;
    }

    return vkGetFenceStatus(m_context.get_device().get_handle(), m_streams[request->m_index].fence) == VK_SUCCESS;
}

std::unique_ptr<vk::Buffer> Streamer::access(StreamHandle* request)
{
    if( !is_loaded(request) )
    {
        return nullptr;
    }

    Stream* stream = &m_streams[request->m_index];

    // return destination buffer
    // Keep this locally so we can free this stream as soon as possible.
    std::unique_ptr<vk::Buffer> retval = std::move(stream->destination);

    free_stream(request->m_index);
    delete request;


    return std::move(retval);
}

void Streamer::discard(StreamHandle* request)
{
    request->m_index = StreamHandle::discard_index;
}

void Streamer::resolve_requests()
{
    // UNFINISHED
    return;

    const u32 max_request_per_resolve = 5;
    u32 resolveCount = 0;
    while( !m_pendingRequests.empty() && resolveCount < max_request_per_resolve )
    {
        i32 nextFreeIdx = get_next_free();
        if( nextFreeIdx == StreamHandle::invalid_index )
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

i32 Streamer::get_next_free()
{
    for( u64 idx = 0; idx < m_state.size(); idx++ )
    {
        if( !m_state[idx] )
        {
            m_state[idx] = true;
            return i32_cast(idx);
        }
    }

    return StreamHandle::invalid_index;
}

void Streamer::free_stream(i32 idx)
{
    Stream* stream = &m_streams[idx];
    CHANNEL_LOG_PROFILE(sys::log::channel::streaming, "Freeing stream of size {} at index {}", stream->source->get_size(), idx);
    stream->source.reset();

    // Assume we've already retrieved the ptr so release just incase.
    stream->destination.release();
    vkResetFences(m_context.get_device().get_handle(), 1, &stream->fence);
    m_state[idx] = false;
}

void Streamer::begin_stream(i32 idx, std::unique_ptr<vk::Buffer>&& source, VkBufferUsageFlags usage)
{
    CHANNEL_LOG_PROFILE(sys::log::channel::streaming, "Beginning stream of size {} at index {}", source->get_size(), idx);

    Stream* stream = &m_streams[u64_cast(idx)];
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