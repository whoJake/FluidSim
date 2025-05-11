#pragma once
#include "types.h"

namespace gfx
{

class memory_info
{
public:
    ~memory_info() = default;
    static memory_info create(void* initial_data, u64 size, format format, memory_type memory_type, resource_type resource_type, u32 usage_flags);

    static memory_info create_as_buffer(void* initial_data, u64 size, format format, memory_type memory_type, buffer_usage_flags usage_flags);
    static memory_info create_as_buffer(u64 size, format format, memory_type memory_type, buffer_usage_flags usage_flags);

    static memory_info create_as_texture(void* initial_data, u64 size, format format, memory_type memory_type, texture_usage_flags usage_flags);
    static memory_info create_as_texture(u64 size, format format, memory_type memory_type, texture_usage_flags usage_flags);

    bool has_initial_data() const;
    void* get_initial_data() const;

    u64 get_size() const;
    format get_format() const;
    memory_type get_memory_type() const;
    resource_type get_resource_type() const;

    buffer_usage_flags get_buffer_usage() const;
    texture_usage_flags get_texture_usage() const;
protected:
    memory_info() = default;
private:
    void* m_initialData;
    u64 m_size;
    format m_format;
    memory_type m_memoryType;
    resource_type m_resourceType;
    union
    {
        buffer_usage_flags m_bufferUsage;
        texture_usage_flags m_textureUsage;
    };
};

} // gfx