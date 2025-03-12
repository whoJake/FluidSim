#pragma once
#include "allocator.h"
#include "dt/hash_string.h"
#include "dt/array.h"
#include <atomic>

namespace sys
{

#define USE_ZONE(id) ::sys::scoped_memory_zone __memzone(id);

enum class budget_failure_type
{
    silent = 0,
    fatal,
};

struct zone_budget
{
    u64 budget;
    budget_failure_type failure_type;
};

using zone_id = u32;

class memory_zone
{
public:
    memory_zone() = default;
    ~memory_zone() = default;

    DEFAULT_MOVE(memory_zone);
    DELETE_COPY(memory_zone);

    void initialise(dt::hash_string32 name, zone_id zone, zone_budget budget);

    zone_id get_id() const;
    bool is_initialised() const;

    void track_allocation(u64 size);
    void track_free(u64 size);

    const dt::hash_string32& get_name() const;
    u64 get_budget() const;
    budget_failure_type get_failure_type() const;

    bool will_exceed_budget(u64 size) const;
private:
    dt::hash_string32 m_name;
    zone_id m_id;
    zone_budget m_budget;
    bool m_initialised{ false };

    std::atomic<u64> m_activeAllocatedMemory{ 0 };
    std::atomic<u64> m_maxActiveAllocatedMemory{ 0 };

    std::atomic<u32> m_activeAllocations{ 0 };
    std::atomic<u32> m_maxActiveAllocations{ 0 };
    std::atomic<u64> m_totalAllocations{ 0 };
    std::atomic<u64> m_totalFreedAllocations{ 0 };
};

enum system_memory_zone : u32
{
    MEMZONE_BEGIN = 0,
    MEMZONE_SYSTEM_BEGIN = MEMZONE_BEGIN,
    
    MEMZONE_SYSTEM_DEFAULT = MEMZONE_SYSTEM_BEGIN,

    MEMZONE_SYSTEM_END,
    MEMZONE_SYSTEM_COUNT = MEMZONE_SYSTEM_END - MEMZONE_SYSTEM_BEGIN,
};

class zone_allocator : public allocator
{
public:
    virtual void* do_allocate(u64 size, u64 align) override;
    virtual void do_free(void* ptr, u64 size) override;

public:
    inline static u64 max_zones = MEMZONE_SYSTEM_COUNT;
    static void initialise_system_zones();

    static void register_zone(dt::hash_string32 name, zone_id id, zone_budget budget);
    static memory_zone* find_memory_zone(zone_id zone);
    static void set_current_zone(zone_id zone);
    static zone_id get_current_zone();

    static zone_allocator* get();
protected:
    void register_zone_internal(dt::hash_string32 name, zone_id id, zone_budget budget);
private:
    thread_local inline static zone_id sm_currentZone = MEMZONE_SYSTEM_DEFAULT;
    dt::array<memory_zone> m_zones;
};

class scoped_memory_zone
{
public:
    scoped_memory_zone(zone_id zone) :
        m_previous(zone_allocator::get_current_zone())
    {
        zone_allocator::set_current_zone(zone);
    }

    ~scoped_memory_zone()
    {
        zone_allocator::set_current_zone(m_previous);
    }
private:
    zone_id m_previous;
};

} // sys