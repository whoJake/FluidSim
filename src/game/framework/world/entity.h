#pragma once

#include "data/transform.h"

namespace fw
{

enum class entity_owner : u8
{
    world = 1 << 0,
};

class entity
{
public:
    entity();
    ~entity();
private:
    mtl::transform m_transform;
};

} // fw