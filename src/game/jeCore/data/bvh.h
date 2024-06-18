#pragma once
#include "aabb.h"

namespace mtl
{

struct bvh_build_settings
{
    size_t maxDepth{ 10 };
    size_t minPayloadsPerNode{ 5 };
};

enum class bvh_split_axis
{
    X,
    Y,
    Z
};

class bvh_payload
{
public:
    virtual ~bvh_payload() = default;
    
    virtual float get_cost() const = 0;
    virtual glm::vec3 get_point() const = 0;
    virtual mtl::aabb3 get_bounds() const = 0;
protected:
    bvh_payload() = default;
    bvh_payload(bvh_payload&&) = default;
    bvh_payload(const bvh_payload&) = default;
    bvh_payload& operator=(bvh_payload&&) = default;
    bvh_payload& operator=(const bvh_payload&) = default;
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
        return count >= 0;
    }
};

template<typename payload>
class fixed_bvh
{
    static_assert(std::is_base_of_v<bvh_payload, payload> == true);
public:
    fixed_bvh() = delete;
    fixed_bvh(const mtl::fixed_vector<payload>& data);
    fixed_bvh(mtl::fixed_vector<payload>&& data);
    fixed_bvh(const std::vector<payload>& data);
    fixed_bvh(std::vector<payload>&& data);

    void build(bvh_build_settings* options);
private:

private:
    std::vector<bvh_node> m_nodes;
    mtl::fixed_vector<payload> m_payloads;
};

} // mtl