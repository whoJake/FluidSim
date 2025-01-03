
#pragma once

#include "gfx_core/fence.h"
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
    u32 aquire_next_image(u64 timeout = u64_max);
    void wait_for_index(u32 index, u64 timeout = u64_max);

    void present(u32 index);

    u32 get_image_count() const;

    const texture* get_image(u32 index) const;
    texture* get_image(u32 index);

    GFX_HAS_IMPL(m_pImpl);
private:
    std::vector<texture> m_images;
    std::vector<fence> m_aquireFences;

    // Lots of unused bits for stuff here.
    u64 m_fenceIndex;
    void* m_pImpl;
};

} // gfx