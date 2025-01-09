#include "swapchain.h"
#ifdef GFX_EXT_SWAPCHAIN

#include "Driver.h"

namespace gfx
{

void swapchain::initialise(std::vector<texture>&& textures, void* pImpl)
{
    m_images = std::move(textures);
    m_pImpl = pImpl;

    m_fenceIndex = 0;
    m_aquireFences.reserve(m_images.size());

    for( u64 i = 0; i < m_images.size(); i++ )
    {
        m_aquireFences.push_back(GFX_CALL(create_fence, false));
    }
}

u32 swapchain::aquire_next_image(u64 timeout)
{
    return GFX_CALL(acquire_next_image, this, &m_aquireFences[m_fenceIndex++], timeout);
}

void swapchain::wait_on_present(u32 index, u64 timeout)
{
    m_aquireFences[index].wait();
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