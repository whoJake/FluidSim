#pragma once
#include "resource_view.h"

namespace gfx
{

class texture_sampler
{
public:
    friend class driver;

    texture_sampler() = default;
    ~texture_sampler() = default;

    DEFAULT_COPY(texture_sampler);
    DEFAULT_MOVE(texture_sampler);

    texture_view* get_texture_view() const;

    GFX_HAS_IMPL(m_pImpl);

    static texture_sampler create(texture_view* view);
    static void destroy(texture_sampler* sampler);
private:
    void* m_pImpl;
    texture_view* m_textureView;
};

} // gfx