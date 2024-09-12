#pragma once

#include <fstream>
#include "path.h"

namespace sys
{

class fi_device
{
public:
    fi_device();
    ~fi_device();

    bool open(const path& uri);
    void close();

    u64 read(u8* dst, u64 size);
    std::vector<u8> read_line();
    std::vector<u8> read_line(char delim);

    void seek_to(u64 byte_offset_from_start);
    void seek(u64 byte_offset);
    void seek_back(u64 byte_offset);

    bool eof() const;

    u64 tell() const;

    bool is_open() const;
    u64 size() const;
private:
    std::ifstream m_handle;
    u64 m_size;
    u64 m_pos;
};

} // sys