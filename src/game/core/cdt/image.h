#pragma once

#include "imageformats.h"

namespace cdt
{

struct image_metadata
{
    image_format format;
    u32 width;
    u32 height;
    u32 depth;
};

enum image_flag_bits : u32
{
    owns_data = 1 << 0,
};

using image_flags = std::underlying_type_t<image_flag_bits>;

class image
{
public:
    image(image_metadata info, image_flags flags, void* data);
    ~image();

    image(const image&);
    image& operator=(const image&);

    DEFAULT_MOVE(image);
    
    bool owns_data() const;
    const image_metadata& get_metadata() const;
    u64 get_size() const;

    void* data();
    void* release_data();
private:
    image_metadata m_metadata;
    image_flags m_flags;
    u32 m_unused{ };
    void* m_data;
};

} // cdt