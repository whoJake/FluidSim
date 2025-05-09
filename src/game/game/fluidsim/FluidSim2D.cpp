#include "FluidSim2D.h"

FluidSim2D::FluidSim2D(FluidSimOptions2D options) :
    m_data(options)
{ }

void FluidSim2D::Simulate(f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces)
{
    for( u64 node_idx = 0; node_idx < m_data.GetNodeCount(); node_idx++ )
    {
        ApplyExternalForces(node_idx, delta_time, external_forces);
    }

    m_data.MoveNodes(delta_time);
}

void FluidSim2D::InsertNode(FluidNodeInfo2D node, glm::f32vec2 position)
{
    m_data.InsertNode(node, position);
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
            }
        }
    }
}