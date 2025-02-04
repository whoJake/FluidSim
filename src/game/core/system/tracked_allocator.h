#pragma once

#include "allocator.h"
#include <vector>

namespace sys
{

using memory_zone = u32;

#define SYSZONE_USE(x) ::sys::tracked_allocator::zone_scope __zonescope(x)
#define SYSZONE_REGISTER(name, val) memory_zone __zone_##val = ::sys::tracked_allocator::register_zone(val, name)
#define SYSZONE_NAME(val) ::sys::tracked_allocator::find_zone_name(val)

enum system_zones : memory_zone
{
    MEMZONE_DEFAULT = 0,
    MEMZONE_SYSTEM,
};

class tracked_allocator : public allocator
{
public:
    using allocate_callback = std::function<void(u64, u64, void*, memory_zone)>;
    using free_callback = std::function<void(void*, memory_zone)>;

    class zone_scope
    {
    public:
        zone_scope(memory_zone target);
        ~zone_scope();
    private:
        memory_zone m_prev;
    };

    static tracked_allocator* get();
    static void set_allocate_callback(allocate_callback cb);
    static void set_free_callback(free_callback cb);
    static memory_zone register_zone(memory_zone val, const char* name);
    static const char* find_zone_name(memory_zone zone);

    void* do_allocate(u64 size, u64 align) override;
    void do_free(void* ptr) override;

    void set_zone(memory_zone value);
    memory_zone get_zone() const;
private:
    tracked_allocator();
    ~tracked_allocator() = default;

    memory_zone* get_thread_zone() const;

    void register_zone_internal(memory_zone val, const char* name);
    const char* find_zone_name_internal(memory_zone val) const;
private:
    allocate_callback m_allocateCb;
    free_callback m_freeCb;

    struct registered_memory_zone
    {
        memory_zone zone;
        const char* name;
    };

    std::vector<registered_memory_zone> m_registeredZones;
};

} // sys