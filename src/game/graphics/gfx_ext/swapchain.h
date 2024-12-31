#pragma once

#include "gfx_core/types.h"
#include "gfx_core/texture.h"
#include "cdt/imageformats.h"

namespace gfx
{

struct surface_capabilities
{
    std::vector<cdt::image_format> formats;
    std::vector<present_mode> present_modes;
    texture_usage_flags supported_usage_flags;
    u32 min_images;
    u32 max_images;
};

class swapchain
{
public:
    swapchain() = default;
    ~swapchain() = default;

    void initialise(std::vector<texture>&& textures, void* pImpl);

    u32 get_image_count() const;

    const texture* get_image(u32 index) const;
    texture* get_image(u32 index);

    template<typename T>
    T get_impl()
    {
        return static_cast<T>(m_impl);
    }
private:
    std::vector<texture> m_images;
    void* m_impl;
};

} // gfx