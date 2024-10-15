#pragma once

namespace gfx
{

enum class memory_type : u32
{
    cpu_accessible,
    gpu_only,
};

struct allocation_info
{
    void* backing_memory;
    u64 size;
    u64 offset;
    u8* mapped;
    u32 type : 2;
    u32 persistant : 1;
    u32 unused : 29;
    u32 padding;

};

class allocator
{
public:
    allocator(void* pImpl = nullptr);
    ~allocator() = default;

    DELETE_COPY(allocator);
    DELETE_MOVE(allocator);

    void set_impl_ptr(void* pImpl);

    void* get_impl_ptr();
    const void* get_impl_ptr() const;
private:
    void* m_impl;
};

} // gfx