#pragma once

#include "data/hash_string.h"
#include "../Component.h"

namespace fw
{

namespace ecs
{

class MeshSource : public Component
{
public:
    MeshSource(const mtl::hash_string& source);
    ~MeshSource() = default;

    DEFAULT_COPY(MeshSource);
    DEFAULT_MOVE(MeshSource);

    const mtl::hash_string& get_source() const;
private:
    mtl::hash_string m_source;
};

} // ecs
} // fw