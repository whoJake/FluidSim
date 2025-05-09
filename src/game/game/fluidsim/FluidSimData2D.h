#pragma once

struct FluidSimOptions2D
{
    glm::vec2 extent;
    glm::vec2 grid_extent;

    bool should_bounce;
    f32 dampening_factor;
};

struct FluidNodeInfo2D
{
    glm::f32vec2 velocity;
    f32 node_radius;
    f32 density;
    f32 mass;

    f32 cell_idx;
    f32 padding[2];
};

class FluidSimData2D
{
public:
    FluidSimData2D(FluidSimOptions2D options);
    ~FluidSimData2D() = default;

    void InsertNode(FluidNodeInfo2D node, glm::f32vec2 position);
    void MoveNodes(f64 delta_time);

    void ClearNodes();

    std::vector<FluidNodeInfo2D>& GetNodeInfos();
    const std::vector<FluidNodeInfo2D>& GetNodeInfos() const;

    const std::vector<glm::f32vec4>& GetNodePositions() const;

    u32 GetNodeCount() const;
private:
    struct GridCell
    {
        std::vector<u64> indices;
    };

    u64 GetCellIndex(glm::f32vec2 position) const;
    void HandleEdge(u64 node_idx);
private:
    FluidSimOptions2D m_options;

    std::vector<glm::f32vec4> m_positions;
    std::vector<FluidNodeInfo2D> m_nodeInfos;

    std::vector<GridCell> m_spatialLookup;
    u32 m_rows;
    u32 m_columns;
};