#include "bvh_split_functions.h"

namespace mtl
{
namespace bvh
{
namespace details
{

bool get_side(const glm::vec3& position, split split)
{
    switch( split.axis )
    {
    case split_axis::X:
        return position.x < split.value;
    case split_axis::Y:
        return position.y < split.value;
    case split_axis::Z:
        return position.z < split.value;
    }

    // problem
    return false;
}

bool get_side(const glm::vec3& position, split_axis axis, const glm::vec3& axisValue)
{
    switch( axis )
    {
    case split_axis::X:
        return get_side(position, { axis, axisValue.x });
    case split_axis::Y:
        return get_side(position, { axis, axisValue.y });
    case split_axis::Z:
        return get_side(position, { axis, axisValue.z });
    }

    // problem
    return false;
}

split half_longest_axis(const aabb3& bounds)
{
    glm::vec3 size = bounds.size();

    if( size.x >= size.y
        && size.x >= size.z )
    {
        return {
            split_axis::X,
            bounds.min.x + bounds.extent().x
        };
    }

    if( size.y >= size.x
        && size.y >= size.z )
    {
        return {
            split_axis::Y,
            bounds.min.y + bounds.extent().y
        };
    }

    return {
        split_axis::Z,
        bounds.min.z + bounds.extent().z
    };
}

template<typename payload>
f32 evaluate_sah(const mtl::fixed_vector<payload>& data, uint32_t dataIdx, uint32_t dataCnt, split_axis axis, glm::vec3 position)
{
    aabb3 left{ aabb3_empty };
    f32 leftCount{ 0 };

    aabb3 right{ aabb3_empty };
    f32 rightCount{ 0 };

    for( u32 i = 0; i < dataCnt; i++ )
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

    f32 cost = left.get_sah_cost() * leftCount + right.get_sah_cost() * rightCount;
    return cost > 0 ? cost : f32_max;
}

template<typename payload>
split optimal_sah(const mtl::fixed_vector<payload>& data, uint32_t dataIdx, uint32_t dataCnt, const aabb3& bounds)
{
    static_assert(std::is_base_of_v<bvh_payload, payload> == true);

    split split{ };
    f32 bestCost = f32_max;

    constexpr split_axis allAxis[3] = { split_axis::X, split_axis::Y, split_axis::Z };

    for( split_axis axis : allAxis )
    {
        for( u32 i = 0; i < dataCnt; i++ )
        {
            const bvh_payload* payload = &data[dataIdx + i];
            f32 cost = evaluate_sah(data, dataIdx, dataCnt, axis, payload->get_point());
            if( cost < bestCost )
            {
                bestCost = cost;
                split.axis = axis;
                switch( axis )
                {
                case split_axis::X:
                    split.value = payload->get_point().x;
                    break;
                case split_axis::Y:
                    split.value = payload->get_point().y;
                    break;
                case split_axis::Z:
                    split.value = payload->get_point().z;
                    break;
                }
            }
        }
    }

    return split;
}

} // details
} // bvh
} // mtl