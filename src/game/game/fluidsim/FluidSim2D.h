#pragma once
#include "glm.hpp"
#include "FluidSimData2D.h"

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

class FluidSim2D
{
public:
    FluidSim2D(FluidSimOptions2D options);
    ~FluidSim2D() = default;

    void Simulate(f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces);
    void InsertNode(FluidNodeInfo2D node, glm::f32vec2 position);

    const std::vector<FluidNodeInfo2D>& GetNodeInfos() const;
    const std::vector<glm::f32vec4>& GetNodePositions() const;

    u32 GetNodeCount() const;

    void Clear();
private:
    void CalculateDensity(u64 node_idx);
    void ApplyExternalForces(u64 node_idx, f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces);
private:
    FluidSimData2D m_data;
};