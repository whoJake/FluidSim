#pragma once

#include "resource.h"
#include "cdt/imageformats.h"
#include "types.h"
#include "gfxdefines.h"

namespace gfx
{

class texture_info
{
public:
    texture_info() = default;
    ~texture_info() = default;

    DEFAULT_MOVE(texture_info);
    DEFAULT_COPY(texture_info);

    void initialise(const texture_info& other);
    void initialise(cdt::image_format format, texture_usage_flags usage, u32 width, u32 height, u32 depthOrLayers);

    cdt::image_format get_format() const;
    u32 get_width() const;
    u32 get_height() const;
    u32 get_depth() const;
    texture_usage_flags get_usage() const;

    u64 get_size() const;
protected:
    cdt::image_format m_format;
    u32 m_width;
    u32 m_height;
    u32 m_depthOrLayers;
    texture_usage_flags m_usage;
};

class texture :
    public resource,
    public texture_info
{
public:
    texture() = default;
    ~texture() = default;

    DEFAULT_MOVE(texture);
    DEFAULT_COPY(texture);

    void initialise(memory_info allocation, texture_info info, void* pImpl, void* pImplView = nullptr, bool isSwapchainImage = false);
    void set_resource_view_type(resource_view_type type, void* pImplView = nullptr);

    bool is_swapchain_image() const;

    GFX_HAS_IMPL(m_pImpl);

    template<typename T>
    T get_view_impl()
    {
        return static_cast<T>(m_pImplView);
    }
private:
    void* m_pImpl;
    void* m_pImplView;
    bool m_isSwapchain;
};

} // gfx