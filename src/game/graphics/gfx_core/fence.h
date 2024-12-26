#pragma once

namespace gfx
{

class fence
{
public:
    fence(void* pImpl = nullptr);
    ~fence();

    bool wait(u64 timeout = u64_max) const;
    bool check() const;
    bool reset();

    template<typename T>
    T get_impl() const
    {
        return static_cast<T>(m_impl);
    }
private:
    void* m_impl;
};

} // gfx