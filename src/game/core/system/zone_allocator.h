#pragma once
#include "allocator.h"
#include "memory_zone.h"
#include "dt/hash_string.h"
#include "dt/array.h"
#include <atomic>

namespace sys
{

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

class memory_zone
{
public:
    memory_zone() = default;
    ~memory_zone() = default;

    DEFAULT_MOVE(memory_zone);
    DELETE_COPY(memory_zone);

    // Name must must be a literal string
    void initialise(const char* name, zone_id zone, zone_budget budget);

    zone_id get_id() const;
    bool is_initialised() const;

    void track_allocation(u64 size);
    void track_free(u64 size);

    const char* get_name() const;
    u64 get_budget() const;
    budget_failure_type get_failure_type() const;

    bool will_exceed_budget(u64 size) const;
private:
    const char* m_name;
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

class zone_allocator : public allocator
{
public:
    virtual void* do_allocate(u64 size, u64 align) override;
    virtual void do_free(void* ptr, u64 size) override;

public:
    inline static u64 max_zones = MEMZONE_SYSTEM_COUNT;

    // Name must be a string literal
    static void register_zone(const char* name, zone_id id, zone_budget budget);
    static memory_zone* find_memory_zone(zone_id zone);
    static void set_current_zone(zone_id zone);
    static zone_id get_current_zone();

    static zone_allocator* get();
protected:
    void register_zone_internal(const char* name, zone_id id, zone_budget budget);
private:

    // This allocator is because we're using a dt::array for the zones. If we didn't do this then
    // destroying the zones during shutdown would attempt to track that allocation which we do not want.
    class untracked_allocator
    {
    public:
        static void* allocate(u64 size, u64 align)
        {
            return _aligned_malloc(size, align);
        }

        static void free(void* ptr, u64 unused)
        {
            return _aligned_free(ptr);
        }
    };

    thread_local inline static zone_id sm_currentZone = MEMZONE_DEFAULT;

    memory_zone m_defaultZone{ };
    dt::array<memory_zone, untracked_allocator> m_zones;
};

} // sys