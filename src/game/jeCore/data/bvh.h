#pragma once
#include "aabb.h"
#include "ray.h"
#include "bvh_payload.h"
#include "details/bvh_node.h"
#include "details/bvh_split_functions.h"

namespace mtl
{

enum class bvh_split_method
{
    HALF_LONGEST_AXIS,
    OPTIMAL_SAH,
};

struct bvh_build_settings
{
    u64 maxDepth{ 10 };
    u64 minPayloadsPerNode{ 5 };
    bvh_split_method split_method{ bvh_split_method::HALF_LONGEST_AXIS };
};

struct bvh_traverse_options
{
    u32 unused;
};

template<typename payload>
struct bvh_traverse_output
{
    const payload* payload_hit;
    f32 distance;
};

struct bvh_traverse_stats
{
    u32 bounces_performed;
    f32 distance_travelled;
    u32 primitives_checked;
    u32 nodes_checked;
};

struct bvh_build_stats
{
    u32 max_depth{ 0 };
    u32 node_count{ 0 };
    u32 max_primitives_in_node{ 0 };
    u32 min_primitives_in_node{ u32_max };
    u32 leaf_node_count{ 0 };
};

template<typename payload>
class fixed_bvh
{
    static_assert(std::is_base_of_v<bvh_payload, payload> == true);
public:
    fixed_bvh() = delete;
    fixed_bvh(const mtl::fixed_vector<payload>& data) :
        m_payloads(data),
        m_nodes({ })
    { }
    fixed_bvh(mtl::fixed_vector<payload>&& data) :
        m_payloads(std::move(data)),
        m_nodes({ })
    { }
    fixed_bvh(const std::vector<payload>& data) :
        m_payloads(data),
        m_nodes({ })
    { }
    fixed_bvh(std::vector<payload>&& data) :
        m_payloads(std::move(data)),
        m_nodes({ })
    { }

    inline bvh_traverse_output<payload> traverse(const ray& target, bvh_traverse_options options, bvh_traverse_stats* out_stats) const
    {
        bvh_traverse_output<payload> retval
        {
            nullptr,
            f32_max
        };

        traverse_impl(target, options, out_stats, &retval, 0);

        return retval;
    }

    inline u32 count_primitives(u32 idx = 0) const
    {
        if( m_nodes[idx].is_leaf() )
        {
            return m_nodes[idx].count;
        }

        return count_primitives(m_nodes[idx].left) + count_primitives(m_nodes[idx].left + 1);
    }

    inline void build(bvh_build_settings* options, bvh_build_stats* stats)
    {
        m_nodes.clear();

        // assign root node.
        bvh::details::node root{ };
        root.left = 0;
        root.count = u32_cast(m_payloads.size());
        m_nodes.push_back(root);

        split(options, 0, 0, stats);
    }
private:
    inline void traverse_impl(const ray& target, bvh_traverse_options options, bvh_traverse_stats* stats, bvh_traverse_output<payload>* output, u32 nodeIndex) const
    {
        stats->nodes_checked++;
        if( !target.intersects(m_nodes[nodeIndex].bounds) )
        {
            return;
        }

        if( m_nodes[nodeIndex].is_leaf() )
        {
            u32 payloadIdx = m_nodes[nodeIndex].left;
            u32 payloadCnt = m_nodes[nodeIndex].count;

            for( uint32_t i = 0; i < m_nodes[nodeIndex].count; i++ )
            {
                stats->primitives_checked++;
                uint32_t pIdx = m_nodes[nodeIndex].left + i;

                ray_hit_info hitInfo{ };
                if( !m_payloads[pIdx].check_ray(target, &hitInfo) )
                {
                    continue;
                }

                if( hitInfo.distance < output->distance )
                {
                    output->payload_hit = &m_payloads[pIdx];
                    output->distance = hitInfo.distance;
                }
            }
        }
        else
        {
            /*
            bool checkFirst = false;

            f32 nodeADist;
            f32 nodeBDist;
            if( target.intersects(m_nodes[m_nodes[nodeIndex].left].bounds, &nodeADist) )
            {
                if( target.intersects(m_nodes[m_nodes[nodeIndex].left + 1].bounds, &nodeBDist) )
                {
                    checkFirst = nodeADist < nodeBDist;
                }

                checkFirst = false;
            }
            else if( target.intersects(m_nodes[m_nodes[nodeIndex].left + 1].bounds, &nodeBDist) )
            {
                checkFirst = true;
            }
            */

            traverse_impl(target, options, stats, output, m_nodes[nodeIndex].left);
            traverse_impl(target, options, stats, output, m_nodes[nodeIndex].left + 1);
        }
    }

    inline void update_node_bounds(u32 nodeIndex)
    {
        bvh::details::node& node = m_nodes[nodeIndex];
        
        if( !node.is_leaf() )
        {
            // error?
            return;
        }

        node.bounds = aabb3_empty;
        for( u32 i = 0; i < node.count; i++ )
        {
            u32 payloadIndex = node.left + i;
            node.bounds.expand_to_fit(m_payloads[payloadIndex].get_bounds());
        }
    }
    
    inline void split(bvh_build_settings* options, u32 nodeIndex, u32 curDepth, bvh_build_stats* stats)
    {
        if( stats )
        {
            stats->node_count++;
            stats->max_depth = std::max(stats->max_depth, curDepth);
        }

        // first, make bounds for this node
        update_node_bounds(nodeIndex);
        
        // At max depth, finish
        if( curDepth == options->maxDepth )
        {
            if( stats )
            {
                stats->min_primitives_in_node = std::min(stats->min_primitives_in_node, m_nodes[nodeIndex].count);
                stats->max_primitives_in_node = std::max(stats->max_primitives_in_node, m_nodes[nodeIndex].count);
                stats->leaf_node_count++;
            }
            return;
        }

        bvh::details::split splitAxis{ };
        switch( options->split_method )
        {
        case bvh_split_method::HALF_LONGEST_AXIS:
            splitAxis = bvh::details::half_longest_axis(m_nodes[nodeIndex].bounds);
            break;
        case bvh_split_method::OPTIMAL_SAH:
            splitAxis = bvh::details::optimal_sah(m_payloads, m_nodes[nodeIndex].left, m_nodes[nodeIndex].count, m_nodes[nodeIndex].bounds);
            break;
        }


        // perform split
        u32 i = m_nodes[nodeIndex].left;
        u32 j = m_nodes[nodeIndex].left + m_nodes[nodeIndex].count - 1;

        u32 k = 0;
        while( i <= j )
        {
            glm::vec3 tmp = m_payloads[i].get_point();
            if( bvh::details::get_side(m_payloads[i].get_point(), splitAxis) )
            {
                i++;
            }
            else
            {
                std::swap( m_payloads[i], m_payloads[j--] );
            }
        }

        u32 leftCount = i - m_nodes[nodeIndex].left;
        u32 rightCount = m_nodes[nodeIndex].count - leftCount;

        // skip split in these cases
        if( leftCount == 0 || leftCount == m_nodes[nodeIndex].count || leftCount <= options->minPayloadsPerNode || rightCount <= options->minPayloadsPerNode )
        {
            if( stats )
            {
                stats->min_primitives_in_node = std::min(stats->min_primitives_in_node, m_nodes[nodeIndex].count);
                stats->max_primitives_in_node = std::max(stats->max_primitives_in_node, m_nodes[nodeIndex].count);
                stats->leaf_node_count++;
            }

            return;
        }

        bvh::details::node left
        {
            { },
            m_nodes[nodeIndex].left,
            leftCount
        };

        bvh::details::node right
        {
            { },
            i,
            m_nodes[nodeIndex].count - leftCount
        };
        
        m_nodes[nodeIndex].left = u32_cast(m_nodes.size());
        m_nodes[nodeIndex].count = 0;

        m_nodes.push_back(left);
        m_nodes.push_back(right);

        split(options, m_nodes[nodeIndex].left, curDepth + 1, stats);
        split(options, m_nodes[nodeIndex].left + 1, curDepth + 1, stats);
    }
private:
    mtl::fixed_vector<payload> m_payloads;
    std::vector<bvh::details::node> m_nodes;
};

} // mtl