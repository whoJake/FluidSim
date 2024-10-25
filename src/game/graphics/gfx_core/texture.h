#pragma once

#include "resource.h"
#include "cdt/imageformats.h"

namespace gfx
{

// Ordered to match VkImageUsageFlagBits
enum texture_usage_flag_bits : u32
{
    texture_transfer_src = 1 << 0,
    texture_transfer_dst = 1 << 1,
    sampled = 1 << 2,
    storage = 1 << 3,
    color = 1 << 4,
    depth_stencil = 1 << 5,
};

using texture_usage_flags = std::underlying_type_t<texture_usage_flag_bits>;

class texture_info
{
public:
    texture_info(cdt::image_format format, texture_usage_flags usage, u32 width, u32 height, u32 depthOrLayers);
    ~texture_info() = default;

    DEFAULT_MOVE(texture_info);
    DEFAULT_COPY(texture_info);

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
    texture(memory_info allocation,
            texture_info info,
            void* pImpl,
            void* pImplView = nullptr);
    ~texture() = default;

    DEFAULT_MOVE(texture);
    DEFAULT_COPY(texture);

    void set_resource_view_type(resource_view_type type, void* pImplView = nullptr);

    template<typename T>
    T get_impl()
    {
        return static_cast<T>(m_impl);
    }

    template<typename T>
    T get_view_impl()
    {
        return static_cast<T>(m_implView);
    }
private:
    void* m_impl;
    void* m_implView;
};

} // gfx