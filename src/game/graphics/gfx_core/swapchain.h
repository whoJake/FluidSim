#pragma once
#include "gfxdefines.h"

#ifdef GFX_EXT_SWAPCHAIN
#include "types.h"
#include "texture.h"
#include "cdt/imageformats.h"
#include "dependency.h"

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

    DEFAULT_COPY(swapchain);
    DEFAULT_MOVE(swapchain);

    void initialise(std::vector<texture>&& textures, void* pImpl);

    swapchain_acquire_result acquire_next_image(u32* aquired_index, dependency* signal_dep, u64 timeout = u64_max);

    void present(u32 index, const std::vector<dependency*>& dependencies = { });

    u32 get_image_count() const;

    const texture* get_image(u32 index) const;
    texture* get_image(u32 index);

    GFX_HAS_IMPL(m_pImpl);
private:
    std::vector<texture> m_images;
    void* m_pImpl;
};

} // gfx
#endif // GFX_EXT_SWAPCHAIN