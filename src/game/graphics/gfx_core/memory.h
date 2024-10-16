#pragma once

namespace gfx
{

enum class memory_type : u32
{
    cpu_accessible,
    gpu_only,
};

struct memory_info
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

} // gfx