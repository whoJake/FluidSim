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

enum class FluidSimExternalDebugType2D
{
    PointPaint = 0,
    SetColor,
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
    void CalculateDensity(u64 node_idx);
    void ApplyExternalForces(u64 node_idx, f64 delta_time, const std::vector<FluidSimExternalForce2D>& external_forces);
    void ApplyExternalDebug(const std::vector<FluidSimExternalDebug2D>& external_debug);
private:
    FluidSimData2D m_data;
};