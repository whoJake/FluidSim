#include "device.h"

namespace sys
{

fi_device::fi_device() :
    m_handle(),
    m_size(),
    m_pos(0)
{ }

fi_device::~fi_device()
{
    close();
}

bool fi_device::open(const path& uri)
{
    if( is_open() )
        close();

    m_handle.open(uri.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
    m_pos = u64_cast(m_handle.tellg());

    if( !m_handle.is_open() )
        return false;

    m_size = tell();
    seek_to(0);
    return true;
}

void fi_device::close()
{
    if( !is_open() )
        return;

    m_handle.close();
}

u64 fi_device::read(u8* dst, u64 size)
{
    if( !is_open() || eof() || m_handle.bad() )
        return 0;

    m_handle.read((char*)dst, size);

    u64 prevPos = m_pos;
    m_pos = std::min(m_pos + size, m_size);
    return m_pos - prevPos;
}

std::vector<u8> fi_device::read_line()
{
    if( !is_open() || eof() || m_handle.bad() )
        return { };

    std::string buf;
    std::getline(m_handle, buf, '\n');

    std::vector<u8> out;
    u64 cpy = buf.size();

    if( buf.back() == '\r' )
        cpy -= 1;

    out.resize(cpy);
    memcpy(out.data(), buf.data(), cpy);

    m_pos = m_handle.eof()
        ? m_size
        : m_pos + out.size();
    return out;
}

std::vector<u8> fi_device::read_line(char delim)
{
    if( !is_open() || eof() || m_handle.bad() )
        return { };

    std::string buf;
    std::getline(m_handle, buf, delim);

    std::vector<u8> out(buf.size());
    memcpy(out.data(), buf.data(), buf.size());

    m_pos = m_handle.eof()
        ? m_size
        : m_pos + out.size();
    return out;

}

void fi_device::seek_to(u64 byte_offset_from_start)
{
    m_pos = byte_offset_from_start;
    // clear must be called before seekg if we're at the end of the file.
    if( m_pos < m_size )
        m_handle.clear();

    m_handle.seekg(byte_offset_from_start);
}

void fi_device::seek(u64 byte_offset)
{
    m_pos += byte_offset;
    // clear must be called before seekg if we're at the end of the file.
    if( m_pos < m_size )
        m_handle.clear();
    else
        m_pos = m_size;

    m_handle.seekg(byte_offset, std::ios_base::right);
}

void fi_device::seek_back(u64 byte_offset)
{
    m_pos -= byte_offset;
    // clear must be called before seekg if we're at the end of the file.
    if( m_pos < m_size )
        m_handle.clear();
    else
        m_pos = m_size;

    m_handle.seekg(byte_offset, std::ios_base::left);
}

bool fi_device::eof() const
{
    return m_pos >= m_size;
}

u64 fi_device::tell() const
{
    return m_pos;
}

bool fi_device::is_open() const
{
    return m_handle.is_open();
}

u64 fi_device::size() const
{
    return m_size;
}

} // sys