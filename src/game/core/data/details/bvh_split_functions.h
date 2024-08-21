#pragma once

#include "data/fixed_vector.h"
#include "data/bvh_payload.h"
#include "bvh_node.h"

namespace mtl
{
namespace bvh
{
namespace details
{

enum class split_axis
{
    X,
    Y,
    Z
};

struct split
{
    split_axis axis;
    f32 value;
};

inline bool get_side(const glm::vec3& position, split split);
inline bool get_side(const glm::vec3& position, split_axis axis, const glm::vec3& axisValue);

inline split half_longest_axis(const aabb3& bounds);

template<typename payload>
inline f32 evaluate_sah(const mtl::fixed_vector<payload>& data, uint32_t dataIdx, uint32_t dataCnt, split_axis axis, glm::vec3 position);

template<typename payload>
inline split optimal_sah(const mtl::fixed_vector<payload>& data, uint32_t dataIdx, uint32_t dataCnt, const aabb3& bounds);

} // details
} // bvh
} // details

#ifndef INC_BVH_SPLIT_FUNCTIONS_INL
#define INC_BVH_SPLIT_FUNCTIONS_INL
#include "bvh_split_functions.inl"
#endif