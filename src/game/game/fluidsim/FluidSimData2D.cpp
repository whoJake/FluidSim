#include "FluidSimData2D.h"
#include "sim_channels.h"

FluidSimData2D::FluidSimData2D(FluidSimOptions2D options) :
    m_options(options)
{
    m_rows = u32_cast(std::ceil(options.extent.x / options.grid_extent.x));
    m_columns = u32_cast(std::ceil(options.extent.y / options.grid_extent.y));

    m_spatialLookup.resize(m_rows * m_columns);
}

void FluidSimData2D::InsertNode(FluidNodeInfo2D node, glm::f32vec2 position)
{
    u64 cell_idx = GetCellIndex(position);
    node.cell_idx = f32_cast(cell_idx) / (m_rows * m_columns);

    m_spatialLookup[cell_idx].indices.push_back(m_positions.size());
    m_positions.push_back({ position.x, position.y, 0.f, 1.f });
    m_nodeInfos.push_back(node);
}

void FluidSimData2D::MoveNodes(f64 delta_time)
{
    for( u64 node_idx = 0; node_idx < m_positions.size(); node_idx++ )
    {
        u64 previous_cell_idx = GetCellIndex(m_positions[node_idx]);

        m_positions[node_idx] += glm::f32vec4(m_nodeInfos[node_idx].velocity * f32_cast(delta_time), 0.f, 0.f);
        HandleEdge(node_idx);

        u64 new_cell_idx = GetCellIndex(m_positions[node_idx]);

        if( previous_cell_idx != new_cell_idx )
        {
            GridCell& prev_cell = m_spatialLookup[previous_cell_idx];

            auto prev_it = std::find(prev_cell.indices.begin(), prev_cell.indices.end(), node_idx);
            FLUIDSIM_ASSERT(prev_it != prev_cell.indices.end(), "Node is being moved from a cell its not inside of?");
            prev_cell.indices.erase(prev_it);
            
            m_spatialLookup[new_cell_idx].indices.push_back(node_idx);

            m_nodeInfos[node_idx].cell_idx = new_cell_idx / (m_rows * m_columns);
        }
    }
}

void FluidSimData2D::ClearNodes()
{
    m_positions.clear();
    m_nodeInfos.clear();

    for( GridCell& cell : m_spatialLookup )
    {
        cell.indices.clear();
    }
}

std::vector<FluidNodeInfo2D>& FluidSimData2D::GetNodeInfos()
{
    return m_nodeInfos;
}

const std::vector<FluidNodeInfo2D>& FluidSimData2D::GetNodeInfos() const
{
    return m_nodeInfos;
}

const std::vector<glm::f32vec4>& FluidSimData2D::GetNodePositions() const
{
    return m_positions;
}

u32 FluidSimData2D::GetNodeCount() const
{
    return u32_cast(m_positions.size());
}

u64 FluidSimData2D::GetCellIndex(glm::f32vec2 position) const
{
    FLUIDSIM_ASSERT(position.x >= 0 && position.x <= m_options.extent.x, "Position is outside the bounds of the simulation.");
    FLUIDSIM_ASSERT(position.y >= 0 && position.y <= m_options.extent.y, "Position is outside the bounds of the simulation.");

    u32 row = std::clamp(u32_cast(std::floor(position.x / m_options.grid_extent.x)), 0u, m_rows - 1u);
    u32 column = std::clamp(u32_cast(std::floor(position.y / m_options.grid_extent.y)), 0u, m_columns - 1u);
    
    return (u64_cast(column) * m_rows) + row;
}

void FluidSimData2D::HandleEdge(u64 node_idx)
{
    glm::f32vec2& position = (glm::f32vec2&)m_positions[node_idx];

    if( m_options.should_bounce )
    {
        glm::f32vec2 velocity_mult{ 1.f, 1.f };

        if( position.x < 0.f )
        {
            position.x *= -1.f;
            velocity_mult.x = -m_options.dampening_factor;
        }
        else if( position.x > m_options.extent.x )
        {
            f32 over = position.x - m_options.extent.x;
            position.x = m_options.extent.x - over;
            velocity_mult.x = -m_options.dampening_factor;
        }

        if( position.y < 0.f )
        {
            position.y *= -1.f;
            velocity_mult.y = -m_options.dampening_factor;
        }
        else if( position.y > m_options.extent.y )
        {
            f32 over = position.y - m_options.extent.y;
            position.y = m_options.extent.y - over;
            velocity_mult.y = -m_options.dampening_factor;
        }

        m_nodeInfos[node_idx].velocity *= velocity_mult;
    }
    else
    {
        // Should not bouce, pass through walls.
        if( position.x < 0.f )
        {
            position.x += m_options.extent.x;
        }
        else if( position.x > m_options.extent.x )
        {
            position.x -= m_options.extent.x;
        }

        if( position.y < 0.f )
        {
            position.y += m_options.extent.y;
        }
        else if( position.y > m_options.extent.y )
        {
            position.y -= m_options.extent.y;
        }
    }
}