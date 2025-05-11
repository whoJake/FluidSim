#include "FluidSimData2D.h"
#include "sim_channels.h"

FluidSimData2D::FluidSimData2D(FluidSimOptions2D options) :
    m_options(options)
{
    m_columns = i32_cast(std::ceil(options.extent.x / options.grid_extent.x));
    m_rows = i32_cast(std::ceil(options.extent.y / options.grid_extent.y));
}

void FluidSimData2D::InsertNode(FluidNodeInfo2D node, glm::f32vec2 position)
{
    m_positions.push_back({ position.x, position.y, 0.f, 1.f });
    m_predictedPositions.push_back(position);
    m_nodeInfos.push_back(node);
}

void FluidSimData2D::MoveNodes(f64 delta_time)
{
    for( u64 node_idx = 0; node_idx < m_positions.size(); node_idx++ )
    {
        m_positions[node_idx] += glm::f32vec4(m_nodeInfos[node_idx].velocity * f32_cast(delta_time), 0.f, 0.f);
        HandleEdge(node_idx);
    }

    // This is soely for our debug features. We shouldn't ever need to build our spatial lookup
    // from anything except the predicted positions.
    BuildSpatialLookup(false);
}

void FluidSimData2D::ClearNodes()
{
    m_positions.clear();
    m_predictedPositions.clear();
    m_nodeInfos.clear();
}

void FluidSimData2D::ForEachNodeInCell(glm::ivec2 cell_coords, ForEachNodeFunc function)
{
    if( cell_coords.x >= m_columns || cell_coords.y >= m_rows )
        return;

    u32 cell_id = GetCellId(cell_coords);
    u32 idx = m_startIndices[cell_id];
    while( idx < GetNodeCount() && m_cellLookup[idx].cell_id == cell_id )
    {
        u32 node_index = m_cellLookup[idx].node_index;
        function(
            m_nodeInfos[node_index],
            m_positions[node_index],
            node_index
        );

        idx++;
    }
}

void FluidSimData2D::ForEachNodeInCell(glm::ivec2 cell_coords, ForEachConstNodeFunc function) const
{
    if( cell_coords.x >= m_columns || cell_coords.y >= m_rows )
        return;

    u32 cell_id = GetCellId(cell_coords);
    u32 idx = m_startIndices[cell_id];
    while( idx < GetNodeCount() && m_cellLookup[idx].cell_id == cell_id )
    {
        u32 node_index = m_cellLookup[idx].node_index;
        function(
            m_nodeInfos[node_index],
            m_positions[node_index],
            node_index
        );

        idx++;
    }
}

void FluidSimData2D::ForEachNodeInCell(glm::f32vec2 sample_point, ForEachNodeFunc function)
{
    ForEachNodeInCell(GetCellCoordinates(sample_point), function);
}

void FluidSimData2D::ForEachNodeInCell(glm::f32vec2 sample_point, ForEachConstNodeFunc function) const
{
    ForEachNodeInCell(GetCellCoordinates(sample_point), function);
}

glm::ivec2 FluidSimData2D::GetCellCoordinates(glm::f32vec2 position) const
{
    return
    {
        i32_cast(std::floor(position.x / m_options.grid_extent.x)),
        i32_cast(std::floor(position.y / m_options.grid_extent.y))
    };
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

const std::vector<glm::f32vec2>& FluidSimData2D::GetNodePredictedPositions() const
{
    return m_predictedPositions;
}

u32 FluidSimData2D::GetNodeCount() const
{
    return u32_cast(m_positions.size());
}

const FluidSimOptions2D& FluidSimData2D::GetOptions() const
{
    return m_options;
}

void FluidSimData2D::FillPredictedPositions()
{
    constexpr f32 const_lookahead_dt = 1.f / 120.f;
    for( u32 node_idx = 0; node_idx < GetNodeCount(); node_idx++ )
    {
        m_predictedPositions[node_idx] = m_positions[node_idx];
        m_predictedPositions[node_idx] += m_nodeInfos[node_idx].velocity * const_lookahead_dt;
    }
}

void FluidSimData2D::BuildSpatialLookup(bool use_predicted_positions)
{
    m_cellLookup.clear();
    m_startIndices.clear();
    m_cellLookup.reserve(GetNodeCount());
    m_startIndices.reserve(GetNodeCount());

    // Build m_cellLookup
    for( u32 node_index = 0; node_index < GetNodeCount(); node_index++ )
    {
        if( use_predicted_positions )
        {
            glm::uvec2 cell_coords = GetCellCoordinates(m_predictedPositions[node_index]);
            m_cellLookup.push_back({ node_index, GetCellId(cell_coords) });
        }
        else
        {
            glm::uvec2 cell_coords = GetCellCoordinates(m_positions[node_index]);
            m_cellLookup.push_back({ node_index, GetCellId(cell_coords) });
        }
    }

    // Sort the cell lookup
    std::sort(m_cellLookup.begin(), m_cellLookup.end(), [](const CellLookup& left, const CellLookup& right)
        {
            return left.cell_id < right.cell_id;
        });

    m_startIndices.resize(GetNodeCount(), invalid_index);
    for( u32 idx = 0; idx < GetNodeCount(); idx++ )
    {
        u32 cell_id = m_cellLookup[idx].cell_id;
        u32 prev_cell_id = idx == 0
            ? invalid_index
            : m_cellLookup[idx - 1].cell_id;

        if( cell_id != prev_cell_id )
            m_startIndices[cell_id] = idx;
    }
}

u32 FluidSimData2D::GetCellId(glm::ivec2 cell_coords) const
{
    static constexpr u32 prime0 = 929;
    static constexpr u32 prime1 = 7127;
    i32 cell_id = cell_coords.x * prime0 + cell_coords.y * prime1;
    return cell_id % GetNodeCount();
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
        else if( position.x >= m_options.extent.x )
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
        else if( position.y >= m_options.extent.y )
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