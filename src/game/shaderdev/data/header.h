#pragma once
#include "dt/vector.h"

constexpr const char* MAGIC_STR = "SFXC";
u32 magic_number();

struct header_desc
{
    u32 EntryPointStartIndex;
    u32 EntryPointSize;
    u32 DataStartIndex;
    u32 DataSize;

    dt::vector<u8> m_heap;
};

void save(const char* filename, const header_desc& hdr);
