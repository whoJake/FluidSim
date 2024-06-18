#pragma once

#include "data/fixed_vector.h"

class Image
{
public:
    Image(void* mapped_region, size_t width, size_t height);
    Image(size_t width, size_t height);
    ~Image();

    glm::vec4 get_pixel(size_t x, size_t y) const;

    void set_pixel(size_t x, size_t y, glm::vec4 value);

    glm::vec4* data() const;

    void dispose();
private:
    size_t get_index_of(size_t x, size_t y) const;
private:
    glm::vec4* m_data;
    size_t m_width;
    size_t m_height;
};
