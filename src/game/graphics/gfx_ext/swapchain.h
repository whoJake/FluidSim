#pragma once

namespace gfx
{

class swapchain
{
public:


    void* get_impl_ptr() const;
private:
    void* m_impl;
};

} // gfx