#pragma once
#include "data/aabb.h"

namespace mtl
{
namespace bvh
{
namespace details
{

struct node
{
    mtl::aabb3 bounds;
    // Represents the left node index when count == 0
    // Represents the payload start index when count > 0
    u32 left;
    u32 count;

    inline bool is_leaf() const
    {
        return count != 0;
    }
};

} // details
} // bvh
} // mtl
