#pragma once

struct FluidSimOptions2D
{
    glm::vec2 extent;
    glm::vec2 grid_extent;

    bool should_bounce;
    f32 dampening_factor;

    f32 smoothing_radius;
    f32 target_density;
    f32 pressure_multiplier;
};

struct FluidNodeInfo2D
{
    glm::f32vec2 velocity;
    f32 node_radius;
    f32 density;
    f32 mass;

    glm::f32vec3 color;
};

class FluidSimData2D
{
public:
    FluidSimData2D(FluidSimOptions2D options);
    ~FluidSimData2D() = default;

    void InsertNode(FluidNodeInfo2D node, glm::f32vec2 position);
    void MoveNodes(f64 delta_time);

    void ClearNodes();

    using ForEachNodeFunc = std::function<void(FluidNodeInfo2D& node_info, const glm::f32vec2 position, u32 node_index)>;
    using ForEachConstNodeFunc = std::function<void(const FluidNodeInfo2D& node_info, const glm::f32vec2 position, u32 node_index)>;

    void ForEachNodeInCell(glm::ivec2 cell_coords, ForEachNodeFunc function);
    void ForEachNodeInCell(glm::ivec2 cell_coords, ForEachConstNodeFunc function) const;

    void ForEachNodeInCell(glm::f32vec2 sample_point, ForEachNodeFunc function);
    void ForEachNodeInCell(glm::f32vec2 sample_point, ForEachConstNodeFunc function) const;

    glm::ivec2 GetCellCoordinates(glm::f32vec2 position) const;

    std::vector<FluidNodeInfo2D>& GetNodeInfos();
    const std::vector<FluidNodeInfo2D>& GetNodeInfos() const;

    const std::vector<glm::f32vec4>& GetNodePositions() const;

    u32 GetNodeCount() const;
    const FluidSimOptions2D& GetOptions() const;

    void BuildSpatialLookup();
private:
    u32 GetCellId(glm::ivec2 cell_coords) const;
    void HandleEdge(u64 node_idx);
private:
    FluidSimOptions2D m_options;

    std::vector<glm::f32vec4> m_positions;
    std::vector<FluidNodeInfo2D> m_nodeInfos;

    struct CellLookup
    {
        u32 node_index;
        u32 cell_id;
    };

    std::vector<CellLookup> m_cellLookup;
    static constexpr u32 invalid_index = u32_max;
    std::vector<u32> m_startIndices;

    i32 m_rows;
    i32 m_columns;
};