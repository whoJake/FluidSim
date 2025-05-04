#include "FluidSim2D.h"
#include "sim_channels.h"

FluidSim2D::FluidSim2D(FluidSimSettings2D settings) :
    m_currentIterationIndex(0)
{
    Initialise(settings);
}

void FluidSim2D::Simulate(f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces)
{
    FluidSimData2D& previous_iteration = m_storedIterations[m_currentIterationIndex];
    m_currentIterationIndex = (m_currentIterationIndex + 1) % u32_cast(m_storedIterations.size());

    m_storedIterations[m_currentIterationIndex].IterateFrom(previous_iteration, delta_time, external_forces);
}

void FluidSim2D::InsertNode(FluidNode2D node)
{
    for( u64 idx = 0; idx < m_storedIterations.size(); idx++ )
    {
        if( !m_storedIterations[idx].InsertNode(node) )
        {
            FLUIDSIM_WARN("Node failed to be inserted into the fluid simulations as it is at capacity.");
            break;
        }
        
    }
}

void FluidSim2D::UpdateSettings(glm::uvec2 dimensions)
{
    SetParameters(dimensions, m_storedIterations[0].GetNodeCapacity());
}

void FluidSim2D::UpdateNodeRadius(f32 radius)
{
    for( u64 idx = 0; idx < m_storedIterations.size(); idx++ )
    {
        FluidNode2D* nodes = m_storedIterations[idx].GetNodes();
        for( u64 node_idx = 0; node_idx < m_storedIterations[idx].GetNodeCount(); node_idx++ )
        {
            nodes[node_idx].node_radius = radius;
        }
    }
}

const FluidSimData2D& FluidSim2D::GetCurrentIterationData() const
{
    return m_storedIterations[m_currentIterationIndex];
}

void FluidSim2D::Initialise(FluidSimSettings2D settings)
{
    // Clear data if there is any
    m_storedIterations.clear();

    // Create our iterations
    m_storedIterations.reserve(2 + settings.additional_previous_iterations);
    for( u64 idx = 0; idx < m_storedIterations.capacity(); idx++ )
        m_storedIterations.push_back(FluidSimData2D(settings.dimensions));

    SetParameters(settings.dimensions, settings.node_capacity);
}

void FluidSim2D::SetParameters(glm::uvec2 dimensions, u32 node_capacity)
{
    for( u64 idx = 0; idx < m_storedIterations.size(); idx++ )
    {
        m_storedIterations[idx].ResizeDimensions(dimensions);
        m_storedIterations[idx].SetNodeCapacity(node_capacity);
    }
}

FluidSimData2D::FluidSimData2D(glm::uvec2 initial_dimensions) :
    m_dimensions(initial_dimensions)
{ }

void FluidSimData2D::ResizeDimensions(glm::uvec2 dimensions)
{
    m_dimensions = dimensions;
}

void FluidSimData2D::IterateFrom(const FluidSimData2D& prev_iteration, f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces)
{
    FLUIDSIM_ASSERT(GetNodeCount() == prev_iteration.GetNodeCount(), "Mismatched numbmer of nodes. Cannot iterate.");

    glm::f32vec2 global_acceleration{ 0.f, 0.f };
    for( const FluidSimExternalForce2D& force : external_forces )
    {
        switch( force.type )
        {
        case FluidSimExternalForceType2D::GravityForce:
            global_acceleration += glm::f32vec2{ 0.f, -force.asGravityForce.acceleration };
            break;
        }
    }

    for( u32 node_idx = 0; node_idx < GetNodeCount(); node_idx++ )
    {
        m_nodes[node_idx].velocity
            = prev_iteration.GetNodes()[node_idx].velocity
            + (global_acceleration * f32_cast(delta_time));

        m_nodes[node_idx].position
            = prev_iteration.GetNodes()[node_idx].position
            + (m_nodes[node_idx].velocity * f32_cast(delta_time));

        
        if( m_nodes[node_idx].position.y - m_nodes[node_idx].node_radius < 0.f )
        {
            f32 dst_over = std::abs(m_nodes[node_idx].position.y - m_nodes[node_idx].node_radius);
            // Bounce
            m_nodes[node_idx].position.y += dst_over;
            m_nodes[node_idx].velocity.x *= 0.9f;
            m_nodes[node_idx].velocity.y *= -0.9f;
        }

        if( m_nodes[node_idx].position.x - m_nodes[node_idx].node_radius < 0.f )
        {
            f32 dst_over = std::abs(m_nodes[node_idx].position.x - m_nodes[node_idx].node_radius);
            m_nodes[node_idx].position.x += dst_over;
            m_nodes[node_idx].velocity.x *= -0.9f;
            m_nodes[node_idx].velocity.y *= 0.9f;
        }

        if( m_nodes[node_idx].position.x + m_nodes[node_idx].node_radius > m_dimensions.x )
        {
            f32 dst_over = std::abs((m_nodes[node_idx].position.x + m_nodes[node_idx].node_radius) - m_dimensions.x);
            m_nodes[node_idx].position.x -= dst_over;
            m_nodes[node_idx].velocity.x *= -0.9f;
            m_nodes[node_idx].velocity.y *= 0.9f;
        }
    }
}

const glm::uvec2& FluidSimData2D::GetDimensions() const
{
    return m_dimensions;
}

void FluidSimData2D::SetNodeCapacity(u32 max_nodes)
{
    if( max_nodes == GetNodeCapacity() )
        return;

    m_nodes.reserve(max_nodes);
}

u32 FluidSimData2D::GetNodeCapacity() const
{
    return u32_cast(m_nodes.capacity());
}

bool FluidSimData2D::InsertNode(FluidNode2D node)
{
    if( m_nodes.size() == m_nodes.capacity() )
        return false;

    m_nodes.push_back(node);
    return true;
}

const FluidNode2D* FluidSimData2D::GetNodes() const
{
    return m_nodes.data();
}

FluidNode2D* FluidSimData2D::GetNodes()
{
    return m_nodes.data();
}

u32 FluidSimData2D::GetNodeCount() const
{
    return u32_cast(m_nodes.size());
}