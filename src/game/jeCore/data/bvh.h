#pragma once
#include "aabb.h"
#include "ray.h"
#include "bvh_payload.h"
#include "implementation/bvh_helpers.h"

namespace mtl
{

enum class bvh_split_method
{
    HALF_LONGEST_AXIS,
    OPTIMAL_SAH,
};

struct bvh_build_settings
{
    size_t maxDepth{ 10 };
    size_t minPayloadsPerNode{ 5 };
    bvh_split_method split_method{ bvh_split_method::HALF_LONGEST_AXIS };
};

struct bvh_traverse_options
{
    uint32_t unused;
};

template<typename payload>
struct bvh_traverse_output
{
    const payload* payload_hit;
    float distance;
};

struct bvh_traverse_stats
{
    uint32_t bounces_performed;
    float distance_travelled;
    uint32_t primitives_checked;
    uint32_t nodes_checked;
};

struct bvh_build_stats
{
    uint32_t max_depth{ 0 };
    uint32_t node_count{ 0 };
    uint32_t max_primitives_in_node{ 0 };
    uint32_t min_primitives_in_node{ std::numeric_limits<uint32_t>::max() };
    uint32_t leaf_node_count{ 0 };
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
            std::numeric_limits<float>::max()
        };

        traverse_impl(target, options, out_stats, &retval, 0);

        return retval;
    }

    inline void build(bvh_build_settings* options, bvh_build_stats* stats)
    {
        m_nodes.clear();

        // assign root node.
        bvh_node root{ };
        root.left = 0;
        root.count = static_cast<uint32_t>(m_payloads.size());
        m_nodes.push_back(root);

        split(options, 0, 0, stats);
    }
private:
    inline void traverse_impl(const ray& target, bvh_traverse_options options, bvh_traverse_stats* stats, bvh_traverse_output<payload>* output, uint32_t nodeIndex) const
    {
        stats->nodes_checked++;
        glm::vec2 intersect = target.intersects(m_nodes[nodeIndex].bounds);

        if( !(intersect.y >= intersect.x && intersect.y > 0) )
        {
            return;
        }

        if( m_nodes[nodeIndex].is_leaf() )
        {
            stats->primitives_checked++;
            uint32_t payloadIdx = m_nodes[nodeIndex].left;
            uint32_t payloadCnt = m_nodes[nodeIndex].count;

            for( uint32_t i = 0; i < m_nodes[nodeIndex].count; i++ )
            {
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
            glm::vec2 n1 = target.intersects(m_nodes[m_nodes[nodeIndex].left].bounds);
            float n1d = n1.x;
            if( n1.y >= n1.x && n1.y > 0 )
            {
                n1d = std::numeric_limits<float>::max();
            }

            glm::vec2 n2 = target.intersects(m_nodes[m_nodes[nodeIndex].left + 1u].bounds);
            float n2d = n2.x;
            if( n2.y >= n2.x && n2.y > 0 )
            {
                n2d = std::numeric_limits<float>::max();
            }

            if( n1d < n2d )
            {
                traverse_impl(target, options, stats, output, m_nodes[nodeIndex].left);
                traverse_impl(target, options, stats, output, m_nodes[nodeIndex].left + 1u);
            }
            else
            {
                traverse_impl(target, options, stats, output, m_nodes[nodeIndex].left + 1u);
                traverse_impl(target, options, stats, output, m_nodes[nodeIndex].left);
            }
        }
    }

    inline void update_node_bounds(uint32_t nodeIndex)
    {
        bvh_node* node = &m_nodes[nodeIndex];
        
        if( node->count == 0 )
        {
            // error?
            return;
        }

        node->bounds = aabb3(m_payloads[node->left].get_bounds());
        for( uint32_t i = 1; i < node->count; i++ )
        {
            uint32_t payloadIndex = node->left + i;
            node->bounds.expand_to_fit(m_payloads[payloadIndex].get_bounds());
        }
    }
    
    inline void split(bvh_build_settings* options, uint32_t nodeIndex, uint32_t curDepth, bvh_build_stats* stats)
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

        bvh_split splitAxis{ };
        switch( options->split_method )
        {
        case bvh_split_method::HALF_LONGEST_AXIS:
            splitAxis = bvh_func::half_bounds(m_nodes[nodeIndex].bounds);
            break;
        case bvh_split_method::OPTIMAL_SAH:
            splitAxis = bvh_func::optimal_sah(m_payloads, m_nodes[nodeIndex].left, m_nodes[nodeIndex].count, m_nodes[nodeIndex].bounds);
            break;
        }


        // perform split
        uint32_t i = m_nodes[nodeIndex].left;
        uint32_t j = m_nodes[nodeIndex].left + m_nodes[nodeIndex].count - 1;

        uint32_t k = 0;
        while( i <= j )
        {
            glm::vec3 tmp = m_payloads[i].get_point();
            if( bvh_func::get_side(m_payloads[i].get_point(), splitAxis) )
            {
                i++;
            }
            else
            {
                std::swap( m_payloads[i], m_payloads[j--] );
            }
        }

        uint32_t leftCount = i - m_nodes[nodeIndex].left;
        uint32_t rightCount = m_nodes[nodeIndex].count - leftCount;

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

        bvh_node left
        {
            { },
            m_nodes[nodeIndex].left,
            leftCount
        };

        bvh_node right
        {
            { },
            i,
            m_nodes[nodeIndex].count - leftCount
        };
        
        m_nodes[nodeIndex].left = static_cast<uint32_t>(m_nodes.size());
        m_nodes[nodeIndex].count = 0;

        m_nodes.push_back(left);
        m_nodes.push_back(right);

        split(options, m_nodes[nodeIndex].left, curDepth + 1, stats);
        split(options, m_nodes[nodeIndex].left + 1, curDepth + 1, stats);
    }
private:
    mtl::fixed_vector<payload> m_payloads;
    std::vector<bvh_node> m_nodes;
};

} // mtl