#pragma once

#include "resource.h"
#include "memory.h"
#include "types.h"
#include "gfxdefines.h"

namespace gfx
{

class buffer : public resource
{
public:
    friend class driver;

    static buffer create(const memory_info& memory_info);
    static void destroy(buffer* buffer);

    buffer() = default;
    ~buffer() = default;

    DEFAULT_MOVE(buffer);
    DEFAULT_COPY(buffer); // TODO can this be deleted?

    // Should the api keep track of all views?
    buffer_view create_view(format format, resource_view_type type, buffer_view_range range = full_buffer_view) const;

    GFX_HAS_IMPL(m_pImpl);
private:
    void* m_pImpl;
};

} // gfx