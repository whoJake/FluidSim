#pragma once

#include "memory.h"
#include "gfxdefines.h"
#include "types.h"
#include "resource_view.h"

namespace gfx
{

class resource
{
public:
    friend class driver;

    resource() = default;
    ~resource() = default;

    DEFAULT_MOVE(resource);
    DEFAULT_COPY(resource);

    void initialise(const memory_info& info, void* backing_memory);

    u64 get_size() const;
    memory_type get_memory_type() const;
    resource_type get_resource_type() const;
    u8* get_mapped() const;

    u8* map();
    void unmap();

    bool is_buffer() const;
    bool is_texture() const;
    bool is_mapped() const;

    template<typename T>
    T get_backing_memory()
    {
        return static_cast<T>(m_backingMemory);
    }

    template<typename T>
    T get_backing_memory() const
    {
        return static_cast<T>(m_backingMemory);
    }
private:
    void* m_backingMemory;
    u64 m_size;
    u8* m_mapped;
    memory_type m_memoryType;
    resource_type m_resourceType;
};

} // gfx