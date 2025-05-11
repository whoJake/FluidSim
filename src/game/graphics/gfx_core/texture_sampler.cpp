#include "texture_sampler.h"
#include "driver.h"

namespace gfx
{

texture_view* texture_sampler::get_texture_view() const
{
    return m_textureView;
}

texture_sampler texture_sampler::create(texture_view* view)
{
    texture_sampler retval{ };
    driver::create_texture_sampler(&retval, view);
    return retval;
}

void texture_sampler::destroy(texture_sampler* sampler)
{
    driver::destroy_texture_sampler(sampler);
}

} // gfx