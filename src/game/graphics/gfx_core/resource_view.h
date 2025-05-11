#pragma once
#include "types.h"
#include "gfxdefines.h"

namespace gfx
{

class resource_view
{
public:
    friend class driver;

    resource_view() = default;
    ~resource_view() = default;

    DEFAULT_MOVE(resource_view);
    DELETE_COPY(resource_view);

    format get_format() const;
    resource_view_type get_type() const;

    GFX_HAS_IMPL(m_pImpl);
protected:
    const void* m_pResource;
private:
    void* m_pImpl;
    format m_format;
    resource_view_type m_type;
    // TODO we should store our view range in here..
};

struct buffer_view_range
{
    u64 offset;
    u64 size;
};
extern buffer_view_range full_buffer_view;

class buffer;
class buffer_view : public resource_view
{
public:
    buffer_view() = default;
    ~buffer_view() = default;

    DEFAULT_MOVE(buffer_view);
    DELETE_COPY(buffer_view);

    const buffer* get_resource() const;

    static buffer_view create(const buffer* buffer, buffer_view_range range, format format = format::UNDEFINED, resource_view_type type = RESOURCE_VIEW_INHERIT);
    static void destroy(buffer_view* buffer_view);
};

// Should include aspect mask in here at somme point aswell as component mappings. idrc for it right now though.
struct texture_view_range
{
    u16 base_mip;
    u16 mip_count;
    u16 base_layer;
    u16 layer_count;
};
extern texture_view_range full_texture_view;

class texture;
class texture_sampler;
class texture_view : public resource_view
{
public:
    texture_view() = default;
    ~texture_view() = default;

    DEFAULT_MOVE(texture_view);
    DELETE_COPY(texture_view);

    const texture* get_resource() const;

    texture_sampler create_sampler();

    static texture_view create(const texture* texture, texture_view_range range, format format = format::UNDEFINED, resource_view_type type = RESOURCE_VIEW_INHERIT);
    static void destroy(texture_view* texture_view);
};

struct texture_attachment
{
    texture_view* view;
    load_operation load;
    store_operation store;
};

} // gfx