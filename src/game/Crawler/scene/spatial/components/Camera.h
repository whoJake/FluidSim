#pragma once

#include "../Component.h"

class CameraComponent : public Component
{
public:
    CameraComponent(float fov = 90.f, float aspect = 1.33f, float nearZ = 0.01f, float farZ = 1000.f);
    ~CameraComponent() = default;

    DEFAULT_COPY(CameraComponent);
    DEFAULT_MOVE(CameraComponent);

    void set_fov(float fov);
    void set_aspect(float aspect);
    void set_near_z(float value);
    void set_far_z(float value);

    glm::mat4 calculate_matrix() const;

    float get_fov() const;
    float get_aspect() const;
    float get_near_z() const;
    float get_far_z() const;
private:
    float m_fov;
    float m_aspect;
    float m_nearZ;
    float m_farZ;
};