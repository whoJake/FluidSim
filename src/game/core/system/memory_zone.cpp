#include "memory_zone.h"
#include "zone_allocator.h"

namespace sys
{

scoped_memory_zone::scoped_memory_zone(zone_id zone) :
    m_previous(zone_allocator::get_current_zone())
{
    zone_allocator::set_current_zone(zone);
}

scoped_memory_zone::~scoped_memory_zone()
{
    zone_allocator::set_current_zone(m_previous);
}

} // sys