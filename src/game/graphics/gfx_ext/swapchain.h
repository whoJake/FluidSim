#pragma once

#include "gfx_core/texture.h"

namespace gfx
{

class swapchain
{
public:
    swapchain(std::vector<texture>&& textures, void* pImpl);
    ~swapchain() = default;

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