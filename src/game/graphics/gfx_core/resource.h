#pragma once

#include "memory.h"

namespace gfx
{

class resource
{
public:
    resource() = default;
    ~resource() = default;

    DEFAULT_MOVE(resource);
    DEFAULT_COPY(resource);

    inline void initialise(memory_info info)
    {
        m_memoryInfo = info;
    }

    template<typename T>
    T get_backing_memory()
    {
        return static_cast<T>(m_memoryInfo.backing_memory);
    }

    inline memory_info& get_memory_info()
    {
        return m_memoryInfo;
    }

    inline const memory_info& get_memory_info() const
    {
        return m_memoryInfo;
    }

    inline u64 get_size() const
    {
        return m_memoryInfo.size;
    }

    inline u32 get_stride() const
    {
        return m_memoryInfo.stride;
    }

    inline resource_view_type get_type() const
    {
        return static_cast<resource_view_type>(m_memoryInfo.type);
    }

    inline bool is_buffer() const
    {
        return get_type() == resource_view_type::buffer;
    }

    inline bool is_texture() const
    {
        return !is_buffer() && (get_type() != resource_view_type::undefined);
    }

    inline bool is_mapped() const
    {
        return m_memoryInfo.mapped != nullptr;
    }

    inline memory_type get_memory_type() const
    {
        return static_cast<memory_type>(m_memoryInfo.type);
    }

    inline bool is_persistant() const
    {
        return m_memoryInfo.persistant;
    }
protected:
    memory_info m_memoryInfo;
};

} // gfx