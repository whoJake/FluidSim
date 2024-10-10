#pragma once

namespace gfx
{

class device
{
public:
    device() = default;
    virtual ~device() = 0;

    virtual void wait_idle() = 0;
private:

};

} // gfx