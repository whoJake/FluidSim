#include "Camera.h"

CameraComponent::CameraComponent(float fov, float aspect, float nearZ, float farZ) :
    m_fov(fov),
    m_aspect(aspect),
    m_nearZ(nearZ),
    m_farZ(farZ)
{ }

void CameraComponent::set_fov(float fov)
{
    m_fov = fov;
}

void CameraComponent::set_aspect(float aspect)
{
    m_aspect = aspect;
}

void CameraComponent::set_near_z(float value)
{
    m_nearZ = value;
}

void CameraComponent::set_far_z(float value)
{
    m_farZ = value;
}

glm::mat4 CameraComponent::calculate_matrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearZ, m_farZ);
}

float CameraComponent::get_fov() const
{
    return m_fov;
}

float CameraComponent::get_aspect() const
{
    return m_aspect;
}

float CameraComponent::get_near_z() const
{
    return m_nearZ;
}

float CameraComponent::get_far_z() const
{
    return m_farZ;
}