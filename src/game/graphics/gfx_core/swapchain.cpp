#include "swapchain.h"
#ifdef GFX_EXT_SWAPCHAIN

#include "Driver.h"

namespace gfx
{

void swapchain::initialise(std::vector<texture>&& textures, void* pImpl)
{
    m_images = std::move(textures);
    m_pImpl = pImpl;
}

swapchain_acquire_result swapchain::acquire_next_image(u32* aquired_index, dependency* signal_dep, u64 timeout)
{
    return GFX_CALL(acquire_next_image, this, aquired_index, signal_dep, nullptr, timeout);
}

void swapchain::present(u32 index, const std::vector<dependency*>& dependencies)
{
    GFX_CALL(present, this, index, dependencies);
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