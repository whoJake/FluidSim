#include "header.h"
#include <fstream>

u32 magic_number()
{
    u8* val = (u8*)&MAGIC_STR;
    return u32_cast(0)
        + (val[0] << 24)
        + (val[1] << 16)
        + (val[2] << 8)
        + (val[3]);
}

void save(const char* filename, const header_desc& hdr)
{
    std::ofstream file(filename, std::ios_base::in | std::ios_base::binary | std::ios_base::trunc);
    file.write(MAGIC_STR, strlen(MAGIC_STR));
    file.write((const char*)&hdr, sizeof(header_desc) - sizeof(dt::vector<u8>));
    file.write((const char*)hdr.m_heap.data(), hdr.m_heap.size());
    file.close();
}