#include "memory.h"

namespace gfx
{

memory_info memory_info::create(void* initial_data,
                                u64 size,
                                format format,
                                memory_type memory_type,
                                resource_type resource_type,
                                u32 usage_flags)
{
    memory_info retval{ };

    retval.m_initialData = initial_data;
    retval.m_size = size;
    retval.m_format = format;
    retval.m_memoryType = memory_type;
    retval.m_resourceType = resource_type;

    switch( resource_type )
    {
        case RESOURCE_TYPE_BUFFER:
            retval.m_bufferUsage = usage_flags;
            break;
        case RESOURCE_TYPE_TEXTURE:
            retval.m_textureUsage = usage_flags;
            break;
        default:
            GFX_ASSERT(false, "Invalid resource type.");
            break;
    }

    return retval;
}

memory_info memory_info::create_as_buffer(void* initial_data,
                                          u64 size,
                                          format format,
                                          memory_type memory_type,
                                          buffer_usage_flags usage_flags)
{
    return create(initial_data, size, format, memory_type, RESOURCE_TYPE_BUFFER, usage_flags);
}

memory_info memory_info::create_as_buffer(u64 size,
                                          format format,
                                          memory_type memory_type,
                                          buffer_usage_flags usage_flags)
{
    return create_as_buffer(nullptr, size, format, memory_type, usage_flags);
}

memory_info memory_info::create_as_texture(void* initial_data,
                                           u64 size,
                                           format format,
                                           memory_type memory_type,
                                           texture_usage_flags usage_flags)
{
    return create(initial_data, size, format, memory_type, RESOURCE_TYPE_TEXTURE, usage_flags);
}

memory_info memory_info::create_as_texture(u64 size,
                                           format format,
                                           memory_type memory_type,
                                           texture_usage_flags usage_flags)
{
    return create_as_texture(nullptr, size, format, memory_type, usage_flags);
}

bool memory_info::has_initial_data() const
{
    return m_initialData != nullptr;
}

void* memory_info::get_initial_data() const
{
    return m_initialData;
}

u64 memory_info::get_size() const
{
    return m_size;
}

format memory_info::get_format() const
{
    return m_format;
}

memory_type memory_info::get_memory_type() const
{
    return m_memoryType;
}

resource_type memory_info::get_resource_type() const
{
    return m_resourceType;
}

buffer_usage_flags memory_info::get_buffer_usage() const
{
    GFX_ASSERT(get_resource_type() == RESOURCE_TYPE_BUFFER, "Accessing buffer usage flags on texture memory.");
    return m_bufferUsage;
}

texture_usage_flags memory_info::get_texture_usage() const
{
    GFX_ASSERT(get_resource_type() == RESOURCE_TYPE_TEXTURE, "Accessing texture usage flags on buffer memory.");
    return m_textureUsage;
}

} // gfx