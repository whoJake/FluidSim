#pragma once

namespace gfx
{

enum class memory_type : u32
{
    cpu_accessible,
    gpu_only,
};

enum class resource_view_type
{
    texture_1d = 0,
    texture_2d,
    texture_3d,
    cube,
    buffer,
    undefined,
};

struct memory_info
{
    void* backing_memory;
    u64 size;
    u8* mapped;
    u32 stride;
    u32 type : 2; // 2
    u32 persistant : 1; // 3
    u32 viewType : 3; // 6
    u32 unused : 26; // 32
};

} // gfx