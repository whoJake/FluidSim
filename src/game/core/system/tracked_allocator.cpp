#include "tracked_allocator.h"

namespace sys
{

tracked_allocator* tracked_allocator::get()
{
    static tracked_allocator instance;
    return &instance;
}

void tracked_allocator::set_allocate_callback(allocate_callback cb)
{
    get()->m_allocateCb = cb;
}

void tracked_allocator::set_free_callback(free_callback cb)
{
    get()->m_freeCb = cb;
}

memory_zone tracked_allocator::register_zone(memory_zone zone, const char* name)
{
    get()->register_zone_internal(zone, name);
    return zone;
}

const char* tracked_allocator::find_zone_name(memory_zone zone)
{
    return get()->find_zone_name_internal(zone);
}

tracked_allocator::tracked_allocator() :
    m_allocateCb([](u64, u64, void*, memory_zone){ }),
    m_freeCb([](void*, memory_zone){ })
{ }

void* tracked_allocator::do_allocate(u64 size, u64 align)
{
    void* ptr = _aligned_malloc(size, align);
    m_allocateCb(size, align, ptr, get_zone());
    return ptr;
}

void tracked_allocator::do_free(void* ptr)
{
    _aligned_free(ptr);
    m_freeCb(ptr, get_zone());
}

void tracked_allocator::set_zone(memory_zone value)
{
    *get_thread_zone() = value;
}

memory_zone tracked_allocator::get_zone() const
{
    return *get_thread_zone();
}

memory_zone* tracked_allocator::get_thread_zone() const
{
    thread_local memory_zone zone = MEMZONE_DEFAULT;
    return &zone;
}

void tracked_allocator::register_zone_internal(memory_zone val, const char* name)
{
    SYSASSERT(find_zone_name_internal(val) == nullptr, SYSMSG_FATAL("Value {} for zone {} is already registered.", val, name));
    m_registeredZones.push_back({ val, name });
}

const char* tracked_allocator::find_zone_name_internal(memory_zone val) const
{
    for( auto it = m_registeredZones.begin(); it != m_registeredZones.end(); ++it )
    {
        if( it->zone == val )
            return it->name;
    }

    return nullptr;
}

tracked_allocator::zone_scope::zone_scope(memory_zone target) :
    m_prev(tracked_allocator::get()->get_zone())
{
    tracked_allocator::get()->set_zone(target);
}

tracked_allocator::zone_scope::~zone_scope()
{
    tracked_allocator::get()->set_zone(m_prev);
}

} // sys