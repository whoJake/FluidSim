#pragma once

#define USE_ZONE(id) ::sys::scoped_memory_zone __memzone(id);

namespace sys
{

using zone_id = u32;

class scoped_memory_zone
{
public:
    scoped_memory_zone(zone_id zone);
    ~scoped_memory_zone();
private:
    zone_id m_previous;
};

} // sys

enum system_memory_zone : u32
{
    MEMZONE_BEGIN = 0,
    MEMZONE_DEFAULT = MEMZONE_BEGIN,

    MEMZONE_SYSTEM_BEGIN = MEMZONE_BEGIN,

    MEMZONE_SYSTEM_HASHSTRING,

    MEMZONE_SYSTEM_END = MEMZONE_SYSTEM_HASHSTRING,
    MEMZONE_SYSTEM_COUNT = MEMZONE_SYSTEM_END - MEMZONE_SYSTEM_BEGIN,
};