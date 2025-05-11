#include "image.h"

namespace cdt
{

image::image(image_metadata info, image_flags flags, void* data) :
    m_metadata(info),
    m_flags(flags),
    m_data(data)
{ }

image::~image()
{ 
    if( m_data && owns_data() )
        delete[] m_data;
}

image::image(const image& other) :
    m_metadata(other.m_metadata),
    m_flags(other.m_flags),
    m_data(nullptr)
{
    if( !other.m_data )
        return;

    m_data = new u8[other.get_size()];
    memcpy(m_data, other.m_data, other.get_size());
    m_flags |= image_flag_bits::owns_data;
}

image& image::operator=(const image& other)
{
    if( m_data && (m_flags & image_flag_bits::owns_data) )
        delete[] m_data;

    m_metadata = other.m_metadata;
    m_flags = other.m_flags;

    if( !other.m_data )
    {
        m_data = nullptr;
        return *this;
    }

    m_data = new u8[other.get_size()];
    memcpy(m_data, other.m_data, other.get_size());
    m_flags |= image_flag_bits::owns_data;
    return *this;
}

bool image::owns_data() const
{
    return m_flags & image_flag_bits::owns_data;
}

const image_metadata& image::get_metadata() const
{
    return m_metadata;
}

u64 image::get_size() const
{
    return u64_cast(get_bits_per_pixel(m_metadata.format) / 8) *
           u64_cast(m_metadata.width) *
           u64_cast(m_metadata.height) *
           u64_cast(m_metadata.depth);
}

void* image::data()
{
    return m_data;
}

void* image::release_data()
{
    if( !m_data || !owns_data() )
        return nullptr;

    m_flags &= ~image_flag_bits::owns_data;
    void* data = m_data;
    m_data = nullptr;
    return data;
}

} // cdt