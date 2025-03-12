#include "zone_allocator.h"

namespace sys
{

void memory_zone::initialise(dt::hash_string32 name, zone_id zone, zone_budget budget)
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
    if( will_exceed_budget(size) )
    {
        if( m_budget.failure_type == budget_failure_type::fatal )
        {
            MEM_ASSERT(false, "Memory zone '{}' cannot allocate {} bytes. Currently allocated: {} Budget: {}", m_name.try_get_str(), size, m_activeAllocatedMemory.load(), m_budget.budget);
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

const dt::hash_string32& memory_zone::get_name() const
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
        MEM_ASSERT(false, "Memory freed from zone {} which was not allocated from this zone.", m_name.try_get_str());

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

void zone_allocator::register_zone(dt::hash_string32 name, zone_id zone, zone_budget budget)
{
    get()->register_zone_internal(name, zone, budget);
}

memory_zone* zone_allocator::find_memory_zone(zone_id zone)
{
    if( get()->m_zones.size() <= zone )
        return nullptr;

    memory_zone* retzone = &(get()->m_zones[zone]);
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

void zone_allocator::initialise_system_zones()
{
    // Id like to do this in the constructor, but we're calling into types that do dynamic allocations (dt::vector / dt::hash_string)
    // which means their constructors will happen before ours and when we destruct, it'll happen AFTER we've destructed the allocator.
    get()->m_zones.initialise(max_zones);
    get()->register_zone_internal(dt::hash_string32("system.default"), MEMZONE_SYSTEM_DEFAULT, { });
}

void zone_allocator::register_zone_internal(dt::hash_string32 name, zone_id zone, zone_budget budget)
{
    MEM_ASSERT(zone <= m_zones.size(), "Zone is outside of valid zone range. Make sure that zone_allocator::max_zones is set to the correct amount before initialising.");
    MEM_ASSERT(!m_zones[zone].is_initialised(), "Double registration of zone {} with id {}", name.try_get_str(), zone);

    m_zones[zone].initialise(name, zone, budget);
}

zone_allocator* zone_allocator::get()
{
    static zone_allocator instance;
    static bool initialised = false;

    if( !initialised ) [[unlikely]]
    {
        initialised = true;
        instance.initialise_system_zones();
    }

    return &instance;
}

} // sys