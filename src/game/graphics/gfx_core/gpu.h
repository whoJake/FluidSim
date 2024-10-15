#pragma once

namespace gfx
{

class gpu
{
public:
    gpu() = default;

    gpu(const char* name,
        u32 index,
        u64 availableMemory,
        bool isDedicated);

    ~gpu() = default;

    DEFAULT_COPY(gpu);
    DEFAULT_MOVE(gpu);

    bool is_dedicated() const;
    u64 get_total_memory() const;

    void* get_impl_ptr() const;
    void set_impl_ptr(void* ptr);
private:
    const char* m_name;
    void* m_impl;
    u64 m_memory;
    u32 m_index;
    bool m_dedicated;
};

} // gfx