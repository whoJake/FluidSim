#include "fiDevice.h"

fiDevice::fiDevice()
{ }

fiDevice::~fiDevice()
{
    try_close();
}

bool fiDevice::open(const char* uri)
{
    try_close();
    return open_drive_file(uri);
}

void fiDevice::close()
{
    try_close();
}

std::vector<uint8_t> fiDevice::read(size_t size)
{
    std::vector<uint8_t> retval;
    retval.resize(size);

    if( !m_handle.is_open() )
    {
        return retval;
    }

    m_handle.read((char*)retval.data(), size);
    return retval;
}

std::vector<uint8_t> fiDevice::read_at(size_t size, size_t offset)
{
    std::vector<uint8_t> retval;
    retval.resize(size);

    if( !m_handle.is_open() )
    {
        return retval;
    }

    m_handle.seekg(offset);
    m_handle.read((char*)retval.data(), size);
    return retval;
}

bool fiDevice::read_line(std::vector<uint8_t>* out, char delimiter, char maybe)
{
    if( !m_handle.is_open() || m_handle.eof() )
    {
        return false;
    }

    std::string str;
    std::getline(m_handle, str, delimiter);
    if( str.size() > 0 && str.at(str.size() - 1) == maybe )
    {
        // remove maybe (mostly for \r)
        str = str.substr(0, str.size() - 1);
    }
    
    *out = std::vector<uint8_t>(str.begin(), str.end());
    return true;
}

size_t fiDevice::get_size() const
{
    return m_size;
}

bool fiDevice::open_drive_file(const char* filepath)
{
    m_handle.open(filepath, std::ios::in | std::ios::ate | std::ios::binary);
    if( !m_handle.is_open() )
    {
        return false;
    }

    m_size = static_cast<size_t>(m_handle.tellg());
    m_handle.seekg(0);
    return true;
}

void fiDevice::try_close()
{
    if( m_handle.is_open() )
    {
        m_handle.close();
    }
}