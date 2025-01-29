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