#include "zone_allocator.h"

namespace sys
{

void memory_zone::initialise(const char* name, zone_id zone, zone_budget budget)
{
    m_name = name;
    m_id = zone;
    m_budget = budget;

    m_initialised = true;
}

zone_id memory_zone::get_id() const
{
    return m_id;
}

bool memory_zone::is_initialised() const
{
    return m_initialised;
}

void memory_zone::track_allocation(u64 size)
{
    if( m_budget.budget != 0
     && will_exceed_budget(size) )
    {
        if( m_budget.failure_type == budget_failure_type::fatal )
        {
            MEM_ASSERT(false, "Memory zone '{}' cannot allocate {} bytes. Currently allocated: {} Budget: {}", m_name, size, m_activeAllocatedMemory.load(), m_budget.budget);
        }
    }

    u64 new_size = m_activeAllocatedMemory.fetch_add(size) + size;
    u64 prev_max = m_maxActiveAllocatedMemory.load();
    while( prev_max < new_size
        && !m_maxActiveAllocatedMemory.compare_exchange_weak(prev_max, new_size) )
    { }

    u32 new_count = m_activeAllocations.fetch_add(1) + 1;
    u32 prev_count = m_maxActiveAllocations.load();
    while( prev_count < new_count
        && !m_maxActiveAllocations.compare_exchange_weak(prev_count, new_count) )
    { }

    m_totalAllocations++;
}

const char* memory_zone::get_name() const
{
    return m_name;
}

u64 memory_zone::get_budget() const
{
    return m_budget.budget;
}

budget_failure_type memory_zone::get_failure_type() const
{
    return m_budget.failure_type;
}

bool memory_zone::will_exceed_budget(u64 size) const
{
    return get_budget() < m_activeAllocatedMemory.load() + size;
}

void memory_zone::track_free(u64 size)
{
    u64 old_active = m_activeAllocatedMemory.fetch_sub(size);
    if( old_active < size )
        MEM_ASSERT(false, "Memory freed from zone {} which was not allocated from this zone.", m_name);

    m_activeAllocations--;
    m_totalFreedAllocations++;
}

void* zone_allocator::do_allocate(u64 size, u64 align)
{
    find_memory_zone(get_current_zone())->track_allocation(size);
    return _aligned_malloc(size, align);
}

void zone_allocator::do_free(void* ptr, u64 size)
{
    _aligned_free(ptr);
    find_memory_zone(get_current_zone())->track_free(size);
}

void zone_allocator::register_zone(const char* name, zone_id zone, zone_budget budget)
{
    get()->register_zone_internal(name, zone, budget);
}

memory_zone* zone_allocator::find_memory_zone(zone_id zone)
{
    if( zone == MEMZONE_DEFAULT )
        return &get()->m_defaultZone;

    u64 zoneIdx = u64_cast(zone) - 1;

    if( get()->m_zones.size() <= zoneIdx )
        return nullptr;

    memory_zone* retzone = &(get()->m_zones[zoneIdx]);
    if( !retzone->is_initialised() )
        return nullptr;

    return retzone;
}

void zone_allocator::set_current_zone(zone_id zone)
{
    sm_currentZone = zone;
}

zone_id zone_allocator::get_current_zone()
{
    return sm_currentZone;
}

void zone_allocator::register_zone_internal(const char* name, zone_id zone, zone_budget budget)
{
    if( zone == 0 && !m_defaultZone.is_initialised() ) [[unlikely]]
    {
        m_defaultZone.initialise(name, zone, budget);
        return;
    }

    MEM_ASSERT(zone != 0, "Double registration of default zone.");
    u64 zoneIdx = u64_cast(zone) - 1;

    MEM_ASSERT(zoneIdx < m_zones.size(), "Zone is outside of valid zone range. Make sure that zone_allocator::max_zones is set to the correct amount before initialising.");
    MEM_ASSERT(!m_zones[zoneIdx].is_initialised(), "Double registration of zone {} with id {}", name, zone);

    m_zones[zoneIdx].initialise(name, zone, budget);
}

zone_allocator* zone_allocator::get()
{
    static zone_allocator instance;
    static bool initialised = false;

    if( !initialised ) [[unlikely]]
    {
        initialised = true;
        instance.m_zones.initialise(max_zones);
    }

    return &instance;
}

} // sys