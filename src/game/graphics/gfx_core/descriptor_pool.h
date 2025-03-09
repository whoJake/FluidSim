#pragma once
#include "shader.h"
#include "dt/hash_string.h"
#include "dt/array.h"
#include "dt/unique_ptr.h"

namespace gfx
{

class descriptor_pool;

/// Descriptor Table : Equal to a descriptor set where the user can bind a set of resources
/// finalise it to write to the underlying API and then bind this descriptor table to a given
/// shader when rendering.
/// TODO: Should this have a concept of being dirty? My thinking is that a descriptor table is
/// flushed at the point it is bound and if its changed then its written to first? Is synchonization
/// something to worry about if I do that?
/// 
/// In Vulkan, a descriptor_table is roughly equivalent to a DescriptorSet and is what binds resources
/// into the pipeline.
class descriptor_table
{
public:
    descriptor_table() = default;
    ~descriptor_table() = default;

    void initialise(descriptor_pool* owner, void* pImpl);

    void set_buffer(dt::hash_string32 name, void* value);
    void set_image(dt::hash_string32 name, void* value);

    const dt::array<void*>& get_buffer_views() const;
    const dt::array<void*>& get_image_views() const;

    const descriptor_table_desc& get_desc() const;

    /// Write the currently applied views into the underlying
    /// descriptor table.
    void write();

    GFX_HAS_IMPL(m_pImpl);
private:
    descriptor_pool* m_owner;

    dt::array<void*> m_bufferViews;
    dt::array<void*> m_imageViews;

    void* m_pImpl;
};

class descriptor_pool
{
public:
    descriptor_pool() = default;
    ~descriptor_pool() = default;

    void initialise(descriptor_table_desc* desc, u32 size);

    descriptor_table* allocate();
    void free(descriptor_table* table);

    void soft_reset();
    void reset();

    const descriptor_table_desc& get_table_desc() const;

    u32 get_capacity() const;

    GFX_HAS_IMPL(m_pImpl);
private:
    descriptor_table* take_available();
    descriptor_table* allocate_from_pool();
private:
    descriptor_table_desc* m_desc;
    dt::vector<dt::unique_ptr<descriptor_table>> m_used;
    dt::vector<dt::unique_ptr<descriptor_table>> m_available;
    u32 m_capacity;
    void* m_pImpl;
};

} // gfx