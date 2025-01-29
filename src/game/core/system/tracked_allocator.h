#pragma once

#include "allocator.h"

namespace sys
{

enum memory_zone
{
    MEMZONE_DEFAULT = 0,

    MEMZONE_POOL,
    MEMZONE_GAME,

    MEMZONE_COUNT,
};

constexpr const char* as_c_str(memory_zone zone)
{
    switch( zone )
    {
    case MEMZONE_DEFAULT:
        return "MEMZONE_DEFAULT";
    case MEMZONE_POOL:
        return "MEMZONE_POOL";
    case MEMZONE_GAME:
        return "MEMZONE_GAME";
    default:
        return "MEMZONE_UNKNOWN";
    }
}

#define SYSUSE_ZONE(x) ::sys::tracked_allocator::zone_scope __zonescope(x)

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

    void* do_allocate(u64 size, u64 align) override;
    void do_free(void* ptr) override;

    void set_zone(memory_zone value);
    memory_zone get_zone() const;

private:
    tracked_allocator();
    ~tracked_allocator() = default;

    memory_zone* get_thread_zone() const;
private:
    allocate_callback m_allocateCb;
    free_callback m_freeCb;
};

} // sys