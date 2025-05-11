#include "FluidSim2D.h"

FluidSim2D::FluidSim2D(FluidSimOptions2D options) :
    m_data(options)
{ }

void FluidSim2D::Simulate(
    f64 delta_time,
    const std::vector<FluidSimExternalForce2D>& external_forces)
{
    for( u64 node_idx = 0; node_idx < m_data.GetNodeCount(); node_idx++ )
    {
        ApplyExternalForces(node_idx, delta_time, external_forces);
        CalculateDensity(node_idx);
    }

    for( u64 node_idx = 0; node_idx < m_data.GetNodeCount(); node_idx++ )
    {
        // Must be done after pre-calculating all the densities
        ApplyPressureForce(node_idx, delta_time);
    }

    m_data.MoveNodes(delta_time);

    // Debugging
    for( FluidNodeInfo2D& node : m_data.GetNodeInfos() )
    {
        node.color = { 1.f, 1.f, 1.f };
    }
}

void FluidSim2D::ApplyDebug(const std::vector<FluidSimExternalDebug2D>& external_debug)
{
    ApplyExternalDebug(external_debug);
}

void FluidSim2D::InsertNode(FluidNodeInfo2D node, glm::f32vec2 position)
{
    m_data.InsertNode(node, position);
}

void FluidSim2D::FinishInserting()
{
    m_data.BuildSpatialLookup();
}

void FluidSim2D::Clear()
{
    m_data.ClearNodes();
}

const std::vector<FluidNodeInfo2D>& FluidSim2D::GetNodeInfos() const
{
    return m_data.GetNodeInfos();
}

const std::vector<glm::f32vec4>& FluidSim2D::GetNodePositions() const
{
    return m_data.GetNodePositions();
}

u32 FluidSim2D::GetNodeCount() const
{
    return m_data.GetNodeCount();
}

f32 FluidSim2D::SmoothingFunction(f32 radius, f32 dst) const
{
    f32 volume = (glm::pi<f32>() * std::pow(radius, 4.f)) / 6.f;
    return (radius - dst) * (radius - dst) / volume;
}

f32 FluidSim2D::SmoothingFunctionDerivitive(f32 radius, f32 dst) const
{
    f32 scale = 12.f / (std::pow(radius, 4.f) * glm::pi<f32>());
    return (dst - radius) * scale;
}

void FluidSim2D::CalculateDensity(u64 node_idx)
{
    // Our grid extent will match our smoothing radius, so we only need to check +-1 around our current cell.
    const std::vector<glm::f32vec4>& node_positions = m_data.GetNodePositions();
    std::vector<FluidNodeInfo2D>& node_infos = m_data.GetNodeInfos();
    FluidNodeInfo2D& current_info = node_infos[node_idx];
    current_info.density = 0;

    glm::f32vec2 node_position{ node_positions[node_idx].x, node_positions[node_idx].y };
    ForEachNodeInRadius(node_position, m_data.GetOptions().smoothing_radius, [&](FluidNodeInfo2D& node, const glm::f32vec2& position, u32 node_index)
        {
            if( node_index == node_idx )
                return;

            f32 distance = glm::length(position - node_position);
            f32 influence = SmoothingFunction(m_data.GetOptions().smoothing_radius, distance);
            current_info.density += current_info.mass * influence;
        });
}

void FluidSim2D::ApplyPressureForce(u64 node_idx, f64 delta_time)
{
    glm::f32vec2 pressure_force{ 0.f, 0.f };
    glm::f32vec2 current_position = m_data.GetNodePositions()[node_idx];

    FluidNodeInfo2D& current_info = m_data.GetNodeInfos()[node_idx];
    if( current_info.density <= glm::epsilon<f32>() )
        return;


    ForEachNodeInRadius(current_position, m_data.GetOptions().smoothing_radius, [&](FluidNodeInfo2D& node, const glm::f32vec2& position, u32 node_index)
        {
            if( node_index == node_idx )
                return;

            f32 distance = glm::length(position - current_position);
            if( distance <= 0.0005f )
                return; // We're too close to accurately calculate forces

            f32 density = m_data.GetNodeInfos()[node_index].density;
            if( density == 0.f )
                return; // Nothing in the smoothing radius. No forces to apply

            glm::f32vec2 direction = (position - current_position) / distance;
            f32 slope = SmoothingFunctionDerivitive(m_data.GetOptions().smoothing_radius, distance);
            f32 mass = m_data.GetNodeInfos()[node_index].mass;
            pressure_force += direction * DensityAsPressure(density) * slope * mass / density;
        });

    current_info.velocity += (pressure_force / current_info.density) * f32_cast(delta_time);
}

void FluidSim2D::ForEachNodeInRadius(glm::f32vec2 sample_point, f32 radius, FluidSimData2D::ForEachNodeFunc function)
{
    glm::ivec2 current_cell = m_data.GetCellCoordinates(sample_point);
    i32 range = i32_cast(std::ceil(radius / std::min(m_data.GetOptions().grid_extent.x, m_data.GetOptions().grid_extent.y)));

    for( i32 cell_y = current_cell.y - range; cell_y <= current_cell.y + range; cell_y++ )
    {
        for( i32 cell_x = current_cell.x - range; cell_x <= current_cell.x + range; cell_x++ )
        {
            glm::ivec2 check_cell{ cell_x, cell_y };
            m_data.ForEachNodeInCell(check_cell, [&](FluidNodeInfo2D& node, const glm::f32vec2& position, u32 node_index)
                {
                    if( glm::length(position - sample_point) > radius )
                        return;

                    function(node, position, node_index);
                });
        }
    }
}

f32 FluidSim2D::DensityAsPressure(f32 density) const
{
    f32 error = density - m_data.GetOptions().target_density;
    return error * m_data.GetOptions().pressure_multiplier;
}

void FluidSim2D::ApplyExternalForces(u64 node_idx, f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces)
{
    for( const FluidSimExternalForce2D& force : external_forces )
    {
        switch( force.type )
        {
            case FluidSimExternalForceType2D::GravityForce:
            {
                FluidSimGravityForce gravity = force.asGravityForce;
                m_data.GetNodeInfos()[node_idx].velocity.y += -gravity.acceleration * f32_cast(delta_time);
                break;
            }
        }
    }
}

void FluidSim2D::ApplyExternalDebug(const std::vector<FluidSimExternalDebug2D>& external_debug)
{
    for( const FluidSimExternalDebug2D& debug : external_debug )
    {
        switch( debug.type )
        {
            case FluidSimExternalDebugType2D::PointPaint:
            {
                FluidSimPointPaint2D paint = debug.asPointPaint;
                // Special case for wanting to only select nodes we're hovered on.
                if( paint.radius == 0.f )
                {
                    m_data.ForEachNodeInCell(paint.position, [&](FluidNodeInfo2D& info, const glm::f32vec2& position, u32 node_index)
                        {
                            if( glm::distance(paint.position, position) <= info.node_radius )
                            {
                                info.color = paint.color;
                            }
                        });
                    break;
                }

                // Otherwise, we have to see how many cells we must check by looking at the radius compared to the cell extent
                i32 search_extent_x = i32_cast(std::ceil(paint.radius / m_data.GetOptions().grid_extent.x));
                i32 search_extent_y = i32_cast(std::ceil(paint.radius / m_data.GetOptions().grid_extent.y));

                glm::ivec2 sample_coords = m_data.GetCellCoordinates(paint.position);
                for( i32 y = sample_coords.y - search_extent_y; y <= sample_coords.y + search_extent_y; y++ )
                {
                    for( i32 x = sample_coords.x - search_extent_x; x <= sample_coords.x + search_extent_x; x++ )
                    {
                        glm::ivec2 coords{ x, y };
                        m_data.ForEachNodeInCell(coords, [&](FluidNodeInfo2D& info, const glm::f32vec2& position, u32 node_index)
                            {
                                if( glm::distance(paint.position, position) <= paint.radius )
                                {
                                    info.color = paint.color;
                                }
                            });
                    }
                }
                break;
            }
            case FluidSimExternalDebugType2D::SetColor:
            {
                FluidSimSetColor set_color = debug.asSetColor;
                for( u32 node_idx = 0; node_idx < m_data.GetNodeCount(); node_idx++ )
                {
                    m_data.GetNodeInfos()[node_idx].color = set_color.color;
                }
                break;
            }
            case FluidSimExternalDebugType2D::SetDensityColor:
            {
                FluidSimSetDensityColor density_color = debug.asDensityColor;
                for( u32 node_idx = 0; node_idx < m_data.GetNodeCount(); node_idx++ )
                {
                    f32 raw_interp = (m_data.GetNodeInfos()[node_idx].density - density_color.min_density) / (density_color.max_density - density_color.min_density);
                    f32 clamped_interp = std::clamp(raw_interp, 0.f, 1.f);
                    m_data.GetNodeInfos()[node_idx].color = density_color.min_color + ((density_color.max_color - density_color.min_color) * clamped_interp);

                    // f32 clamped_interp = (std::clamp(DensityAsPressure(m_data.GetNodeInfos()[node_idx].density), -1.f, 1.f) + 1) / 2.f;
                    // m_data.GetNodeInfos()[node_idx].color = density_color.min_color + ((density_color.max_color - density_color.min_color) * clamped_interp);
                }
                break;
            }
        }
    }
}