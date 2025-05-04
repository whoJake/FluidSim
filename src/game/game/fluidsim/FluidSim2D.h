#pragma once
#include "glm.hpp"

struct FluidNode2D
{
    glm::f32vec2 position;
    glm::f32vec2 velocity;
    f32 node_radius;
    f32 padding[3];
};

struct FluidSimGravityForce
{
    f32 acceleration;
};

enum class FluidSimExternalForceType2D
{
    GravityForce = 0,
};

struct FluidSimExternalForce2D
{
    FluidSimExternalForceType2D type;
    union
    {
        FluidSimGravityForce asGravityForce;
    };
};

class FluidSimData2D
{
public:
    FluidSimData2D(glm::uvec2 initial_dimensions);
    ~FluidSimData2D() = default;

    void IterateFrom(const FluidSimData2D& prev_iteration, f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces);

    void ResizeDimensions(glm::uvec2 dimensions);
    const glm::uvec2& GetDimensions() const;

    void SetNodeCapacity(u32 max_nodes);
    u32 GetNodeCapacity() const;

    bool InsertNode(FluidNode2D node);

    FluidNode2D* GetNodes();
    const FluidNode2D* GetNodes() const;
    u32 GetNodeCount() const;
private:
    glm::uvec2 m_dimensions;
    std::vector<FluidNode2D> m_nodes;
};

struct FluidSimSettings2D
{
    glm::uvec2 dimensions;
    u32 node_capacity;
    u32 additional_previous_iterations;
};

class FluidSim2D
{
public:
    FluidSim2D(FluidSimSettings2D settings);
    ~FluidSim2D() = default;

    void Simulate(f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces);
    void InsertNode(FluidNode2D node);

    void UpdateSettings(glm::uvec2 dimensions);
    void UpdateNodeRadius(f32 radius);

    const FluidSimData2D& GetCurrentIterationData() const;
private:
    void Initialise(FluidSimSettings2D settings);
    void SetParameters(glm::uvec2 dimensions, u32 node_capacity);
private:
    std::vector<FluidSimData2D> m_storedIterations;
    u32 m_currentIterationIndex;
};