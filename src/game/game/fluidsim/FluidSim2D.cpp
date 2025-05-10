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

void FluidSim2D::CalculateDensity(u64 node_idx)
{

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
        }
    }
}