#include "Image.h"

Image::Image(void* mapped_region, size_t width, size_t height) :
    m_data(static_cast<glm::vec4*>(mapped_region)),
    m_width(width),
    m_height(height)
{ }

Image::Image(size_t width, size_t height) :
    m_data(new glm::vec4[width * height]),
    m_width(width),
    m_height(height)
{ }

Image::~Image()
{
}

glm::vec4 Image::get_pixel(size_t x, size_t y) const
{
    return m_data[get_index_of(x, y)];
}

void Image::set_pixel(size_t x, size_t y, glm::vec4 value)
{
    m_data[get_index_of(x, y)] = value;
}

glm::vec4* Image::data() const
{
    return m_data;
}

size_t Image::get_index_of(size_t x, size_t y) const
{
    return y * m_width + x;
}

void Image::dispose()
{
    delete[] m_data;
}