#pragma once

#include "cdt/image.h"
#include "system/path.h"

namespace cdt
{

class image_loader
{
public:
    static std::unique_ptr<image> from_file_png(sys::path path);
    static std::unique_ptr<image> from_memory_png(void* pData, u64 size);
public:
    image_loader() = delete;
    ~image_loader() = delete;
};

} // cdt