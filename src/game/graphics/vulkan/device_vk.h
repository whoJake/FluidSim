#pragma once
#include "gfx_core/device.h"

namespace gfx
{

class device_vk : public device
{
public:
    device_vk();
    virtual ~device_vk();

    void wait_idle() override;
private:

};

} // gfx