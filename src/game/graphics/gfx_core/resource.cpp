#include "resource.h"
#include "driver.h"

namespace gfx
{

void resource::initialise(const memory_info& info, void* backing_memory)
{
    m_backingMemory = backing_memory;
    m_size = info.get_size();
    m_mapped = nullptr;
    m_memoryType = info.get_memory_type();
    m_resourceType = info.get_resource_type();
}

u64 resource::get_size() const
{
    return m_size;
}

memory_type resource::get_memory_type() const
{
    return m_memoryType;
}

resource_type resource::get_resource_type() const
{
    return m_resourceType;
}

u8* resource::get_mapped() const
{
    return m_mapped;
}

u8* resource::map()
{
    driver::map_resource(this);
    return get_mapped();
}

void resource::unmap()
{
    driver::unmap_resource(this);
}

bool resource::is_buffer() const
{
    return m_resourceType == RESOURCE_TYPE_BUFFER;
}

bool resource::is_texture() const
{
    return m_resourceType == RESOURCE_TYPE_TEXTURE;
}

bool resource::is_mapped() const
{
    return m_mapped != nullptr;
}

} // gfx