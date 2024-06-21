#pragma once
#include "aabb.h"
#include "ray.h"

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

    virtual bool check_ray(const ray& ray, ray_hit_info* hit) const = 0;
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

struct bvh_traverse_options
{
    uint32_t max_bounces;
};

struct bvh_traverse_output
{
    glm::vec3 diffuse;
    glm::vec3 initial_normal;
    glm::vec3 final_normal;
};

struct bvh_traverse_stats
{
    uint32_t bounces_performed;
    float distance_travelled;
    uint32_t primitives_checked;
};

template<typename payload>
class fixed_bvh
{
    static_assert(std::is_base_of_v<bvh_payload, payload> == true);
public:
    fixed_bvh() = delete;
    fixed_bvh(const mtl::fixed_vector<payload>& data) :
        m_payloads(data),
        m_nodes({ initial_node() })
    { }
    fixed_bvh(mtl::fixed_vector<payload>&& data) :
        m_payloads(std::move(data)),
        m_nodes({ initial_node() })
    { }
    fixed_bvh(const std::vector<payload>& data) :
        m_payloads(data),
        m_nodes({ initial_node() })
    { }
    fixed_bvh(std::vector<payload>&& data) :
        m_payloads(std::move(data)),
        m_nodes({ initial_node() })
    { }

    inline bvh_traverse_output traverse(const ray& target, bvh_traverse_options options, bvh_traverse_stats* out_stats) const
    {
        bvh_traverse_output output{ };
        bool hasOutput = false;

        ray next_ray = target;
        for( uint32_t i = 0; i < options.max_bounces; i++ )
        {
            ray_hit_info bestHit{ };
            bestHit.distance = std::numeric_limits<float>::max();

            // naive, check all
            for( const payload& payload : m_payloads )
            {
                ray_hit_info hit{ };
                if( payload.check_ray(next_ray, &hit) )
                {
                    if( hit.distance < bestHit.distance )
                    {
                        bestHit = hit;
                    }
                }
            }

            // Stop bouncing if distance is unchanged
            if( bestHit.distance == std::numeric_limits<float>::max() )
            {
                break;
            }

            if( out_stats )
            {
                out_stats->bounces_performed++;
                out_stats->distance_travelled += bestHit.distance;
                out_stats->primitives_checked += static_cast<uint32_t>(m_payloads.size());
            }

            glm::vec3 sun = glm::normalize(glm::vec3{ 0.4f, -1.f, -.2f });
            float strength = glm::clamp(glm::dot(sun, bestHit.normal), 0.1f, 1.f);

            // Change output color
            if( !hasOutput )
            {
                output.diffuse = bestHit.diffuse * strength;
                hasOutput = true;
            }
            else
            {
                output.diffuse = (output.diffuse + bestHit.diffuse * strength) / 2.f;
            }

            // set normal always, it'll stay as the last one when we finish
            output.final_normal = bestHit.normal;

            if( i == 0 )
            {
                output.initial_normal = bestHit.normal;
            }

            glm::vec3 oldDirection = next_ray.direction;
            next_ray.position = next_ray.position + (oldDirection * bestHit.distance);
            next_ray.direction = glm::reflect(oldDirection, bestHit.normal);
        }

        return output;
    }

    inline void build(bvh_build_settings* options)
    { }
private:
    inline bvh_node initial_node() const
    {
        if( m_payloads.size() < 0 )
            return{ };

        mtl::aabb3 bounds = m_payloads[0].get_bounds();
        for( size_t i = 1; i < m_payloads.size(); i++ )
        {
            bounds.expand_to_fit(m_payloads[i].get_bounds());
        }

        return
        {
            bounds,
            0,
            static_cast<uint32_t>(m_payloads.size())
        };
    }
private:
    mtl::fixed_vector<payload> m_payloads;
    std::vector<bvh_node> m_nodes;
};

} // mtl