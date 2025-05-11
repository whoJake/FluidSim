#pragma once

#include "image_loaders.h"
#include "system/device.h"

#include "cdt/image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
//#define STBI_MALLOC(sz) do_stbi_malloc(sz)
//#define STBI_REALLOC(p, newsz) do_stbi_realloc(p, newsz)
//#define STBI_FREE(p) do_stbi_free(p)
#include "stb_image.h"
#include "stb_image_write.h"

namespace cdt
{

std::unique_ptr<image> image_loader::from_memory_png(void* data, u64 size)
{
    i32 width, height, channels;
    stbi_uc* img = stbi_load_from_memory((stbi_uc*)data, u32_cast(size), &width, &height, &channels, STBI_rgb_alpha);

    if( !img )
        return nullptr;

    image_metadata metadata
    {
        image_format::R8G8B8A8_SRGB,
        u32_cast(width),
        u32_cast(height),
        u32_cast(1),
    };

    image_flags flags = image_flag_bits::owns_data;

    return std::make_unique<image>(metadata, flags, img);
}

std::unique_ptr<image> image_loader::from_file_png(sys::path path)
{
    sys::fi_device device;
    device.open(path);
    u64 filesize = device.size();
    u8* data = new u8[filesize];
    device.read(data, filesize);
    device.close();

    std::unique_ptr<image> retval = image_loader::from_memory_png(data, filesize);
    return retval;
}

} // cdt