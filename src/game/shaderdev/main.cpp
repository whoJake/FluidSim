#include "data/header.h"
#include "system/tracked_allocator.h"

int main(int argc, const char* argv[])
{
    sys::allocator::set_main(sys::tracked_allocator::get());

    const char* val = "TESTSTR";
    header_desc hdr;
    hdr.EntryPointStartIndex = 0;
    hdr.EntryPointSize = strlen(val);
    hdr.DataStartIndex = hdr.EntryPointSize;
    hdr.DataSize = 8;

    const char* two = val;
    while( *two )
    {
        hdr.m_heap.push_back((u8)*two);
        ++two;
    }

    hdr.m_heap.push_back(u8_max);
    hdr.m_heap.push_back(u8_max);
    hdr.m_heap.push_back(u8_max);
    hdr.m_heap.push_back(u8_max);

    save("c:/Users/Jake/Desktop/save.fx", hdr);
}