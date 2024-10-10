#pragma once

namespace gfx
{

enum class memory_type : u32
{
    cpu_accessable,
    gpu_only,
};

class allocation_info
{
public:
private:
    void* m_memory;
    u64 m_size;
    u8* m_mapped;
    u32 m_type : 2;
    u32 m_persistant : 1;
    u32 m_unused : 29;
    u32 m_padding;

};

class texture
{
public:
private:
};

class buffer
{
public:
private:
};

} // gfx