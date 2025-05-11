#pragma once
#include "glm.hpp"
#include "FluidSimData2D.h"

struct FluidSimGravityForce
{
    f32 acceleration;
};

struct FluidSimPointForce2D
{
    glm::f32vec2 position;
    f32 radius;
    f32 force;
};

enum class FluidSimExternalForceType2D
{
    GravityForce = 0,
    PointForce,
};

struct FluidSimPointPaint2D
{
    glm::f32vec2 position;
    f32 radius;
    glm::f32vec3 color;
};

struct FluidSimSetColor
{
    glm::f32vec3 color;
};

struct FluidSimSetDensityColor
{
    f32 min_density;
    f32 max_density;
    glm::f32vec3 min_color;
    glm::f32vec3 max_color;
};

struct FluidSimSetVelocityColor
{
    f32 min_velocity;
    f32 max_velocity;
    glm::f32vec3 min_color;
    glm::f32vec3 max_color;
};

enum class FluidSimExternalDebugType2D
{
    PointPaint = 0,
    SetColor,
    SetDensityColor,
    SetVelocityColor,
};

struct FluidSimExternalForce2D
{
    FluidSimExternalForceType2D type;
    union
    {
        FluidSimGravityForce asGravityForce;
        FluidSimPointForce2D asPointForce;
    };
};

struct FluidSimExternalDebug2D
{
    FluidSimExternalDebugType2D type;
    union
    {
        FluidSimPointPaint2D asPointPaint;
        FluidSimSetColor asSetColor;
        FluidSimSetDensityColor asDensityColor;
        FluidSimSetVelocityColor asVelocityColor;
    };
};

class FluidSim2D
{
public:
    FluidSim2D(FluidSimOptions2D options);
    ~FluidSim2D() = default;

    void Simulate(
        f64 delta_time,
        const std::vector<FluidSimExternalForce2D>& external_forces = { });

    void ApplyDebug(const std::vector<FluidSimExternalDebug2D>& external_debug);

    void InsertNode(FluidNodeInfo2D node, glm::f32vec2 position);
    void FinishInserting();

    const std::vector<FluidNodeInfo2D>& GetNodeInfos() const;
    const std::vector<glm::f32vec4>& GetNodePositions() const;

    u32 GetNodeCount() const;

    void Clear();
private:
    f32 SmoothingFunction(f32 radius, f32 dst) const;
    f32 SmoothingFunctionDerivitive(f32 radius, f32 dst) const;

    void CalculateDensity(u64 node_idx);
    void ApplyPressureForce(u64 node_idx, f64 delta_time);

    f32 DensityAsPressure(f32 density) const;

    void ForEachNodeInRadius(glm::f32vec2 sample_point, f32 radius, FluidSimData2D::ForEachNodeFunc function);
    void ApplyExternalForces(u64 node_idx, f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces);
    void ApplyExternalDebug(const std::vector<FluidSimExternalDebug2D>& external_debug);
private:
    FluidSimData2D m_data;
};