#pragma once

#include "data/fixed_vector.h"
#include "data/bvh_payload.h"
#include "data/aabb.h"

namespace mtl
{

enum class bvh_split_axis
{
    X,
    Y,
    Z
};

struct bvh_split
{
    bvh_split_axis axis;
    float value;
};

struct bvh_node
{
    mtl::aabb3 bounds;
    // Represents the left node index when count == 0
    // Represents the payload start index when count > 0
    uint32_t left;
    uint32_t count;

    inline bool is_leaf() const
    {
        return count != 0;
    }
};

namespace bvh_func
{

static bool get_side(const glm::vec3& position, bvh_split split)
{
    switch( split.axis )
    {
    case bvh_split_axis::X:
        return position.x < split.value;
    case bvh_split_axis::Y:
        return position.y < split.value;
    case bvh_split_axis::Z:
        return position.z < split.value;
    }

    // problem
    return false;
}

static bool get_side(const glm::vec3& position, bvh_split_axis axis, const glm::vec3& axisValue)
{
    switch( axis )
    {
    case bvh_split_axis::X:
        return get_side(position, { axis, axisValue.x });
    case bvh_split_axis::Y:
        return get_side(position, { axis, axisValue.y });
    case bvh_split_axis::Z:
        return get_side(position, { axis, axisValue.z });
    }

    // problem
    return false;
}

static bvh_split half_bounds(const aabb3& bounds)
{
    glm::vec3 size = bounds.size();

    if( size.x >= size.y
        && size.x >= size.z )
    {
        return {
            bvh_split_axis::X,
            bounds.min.x + bounds.extent().x
        };
    }

    if( size.y >= size.x
        && size.y >= size.z )
    {
        return {
            bvh_split_axis::Y,
            bounds.min.y + bounds.extent().y
        };
    }

    return {
        bvh_split_axis::Z,
        bounds.min.z + bounds.extent().z
    };
}

template<typename payload>
static float evaluate_sah(const mtl::fixed_vector<payload>& data, uint32_t dataIdx, uint32_t dataCnt, bvh_split_axis axis, glm::vec3 position)
{
    aabb3 left{ aabb3_empty };
    float leftCount{ 0 };

    aabb3 right{ aabb3_empty };
    float rightCount{ 0 };

    for( uint32_t i = 0; i < dataCnt; i++ )
    {
        const bvh_payload* payload = &data[dataIdx + i];
        bool side = get_side(payload->get_point(), axis, position);

        if( side )
        {
            leftCount++;
            left.expand_to_fit(payload->get_bounds());
        }
        else
        {
            rightCount++;
            right.expand_to_fit(payload->get_bounds());
        }
    }

    float cost = left.get_sah_cost() * leftCount + right.get_sah_cost() * rightCount;
    return cost > 0 ? cost : std::numeric_limits<float>::max();
}

template<typename payload>
static bvh_split optimal_sah(const mtl::fixed_vector<payload>& data, uint32_t dataIdx, uint32_t dataCnt, const aabb3& bounds)
{
    static_assert(std::is_base_of_v<bvh_payload, payload> == true);

    bvh_split split{ };
    float bestCost = std::numeric_limits<float>::max();

    constexpr bvh_split_axis allAxis[3] = { bvh_split_axis::X, bvh_split_axis::Y, bvh_split_axis::Z };

    for( bvh_split_axis axis : allAxis )
    {
        for( uint32_t i = 0; i < dataCnt; i++ )
        {
            const bvh_payload* payload = &data[dataIdx + i];
            float cost = evaluate_sah(data, dataIdx, dataCnt, axis, payload->get_point());
            if( cost < bestCost )
            {
                bestCost = cost;
                split.axis = axis;
                switch( axis )
                {
                case bvh_split_axis::X:
                    split.value = payload->get_point().x;
                    break;
                case bvh_split_axis::Y:
                    split.value = payload->get_point().y;
                    break;
                case bvh_split_axis::Z:
                    split.value = payload->get_point().z;
                    break;
                }
            }
        }
    }

    return split;
}

/*
template<typename payload>
static bvh_split estimate_optimal_sah(const mtl::fixed_vector<payload>& data, uint32_t dataIdx, uint32_t dataCnt, const aabb3& bounds, uint32_t resolution)
{
    static_assert(std::is_base_of_v<bvh_payload, payload> == true);

    bvh_split split{ };
    float bestCost = std::numeric_limits<float>::max();

    constexpr bvh_split_axis allAxis[3] = { bvh_split_axis::X, bvh_split_axis::Y, bvh_split_axis::Z };

    for( bvh_split_axis axis : allAxis )
    {
        float axisStepResolution{ 0.f };
        switch( axis )
        {
        case bvh_split_axis::X:
            axisStepResolution = bounds.size().x / resolution;
            break;
        case bvh_split_axis::Y:
            axisStepResolution = bounds.size().y / resolution;
            break;
        case bvh_split_axis::Z:
            axisStepResolution = bounds.size().z / resolution;
            break;

        }

        for( uint32_t i = 1; i < resolution; i++ )
        {
            const bvh_payload* payload = &data[dataIdx + i];
            float cost = evaluate_sah(data, dataIdx, dataCnt, axis, payload->get_point());
            if( cost < bestCost )
            {
                bestCost = cost;
                split.axis = axis;
                switch( axis )
                {
                case bvh_split_axis::X:
                    split.value = payload->get_point().x;
                    break;
                case bvh_split_axis::Y:
                    split.value = payload->get_point().y;
                    break;
                case bvh_split_axis::Z:
                    split.value = payload->get_point().z;
                    break;
                }
            }
        }
    }

    return split;
}
*/

} // bvh_func
} // mtl