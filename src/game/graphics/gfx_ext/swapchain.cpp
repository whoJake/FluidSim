#include "swapchain.h"
#include "gfx_core/gfxdefines.h"

#ifdef GFX_EXT_SWAPCHAIN
namespace gfx
{

void swapchain::initialise(std::vector<texture>&& textures, void* pImpl)
{
    m_images = std::move(textures);
    m_impl = pImpl;
}

u32 swapchain::get_image_count() const
{
    return u32_cast(m_images.size());
}

const texture* swapchain::get_image(u32 index) const
{
    return &m_images[index];
}

texture* swapchain::get_image(u32 index)
{
    return &m_images[index];
}

} // gfx
#endif // GFX_EXT_SWAPCHAIN