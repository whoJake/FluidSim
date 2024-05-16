#include "Camera.h"

Camera::Camera() :
    m_position(0.f, 0.f, 0.f),
    m_rotation(glm::quat(glm::vec3(0.f)))
{ }

void Camera::rotate(const glm::vec3& eulerRotation)
{
    // broken
    rotate(glm::angleAxis(eulerRotation.z, glm::vec3(0.f, 0.f, 1.f)));
    rotate(glm::angleAxis(eulerRotation.y, glm::vec3(0.f, 1.f, 0.f)));
    rotate(glm::angleAxis(eulerRotation.x, glm::vec3(1.f, 0.f, 0.f)));
}

void Camera::rotate(const glm::quat& rotation)
{
    m_rotation *= rotation;
}

void Camera::translate(const glm::vec3& translation)
{
    if( translation.length() == 0.f )
    {
        return;
    }

    m_position += translation;
}

glm::vec3 Camera::get_position() const
{
    return m_position;
}

glm::vec3 Camera::get_euler_rotation() const
{
    return glm::eulerAngles(m_rotation);
}

glm::quat Camera::get_rotation() const
{
    return m_rotation;
}

glm::mat4 Camera::as_view_matrix() const
{
    return glm::toMat4(m_rotation) * glm::translate(glm::mat4(1.f), -m_position);
}

PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float nearZ, float farZ) :
    Camera(),
    m_fov(fov),
    m_aspect(aspect),
    m_nearZ(nearZ),
    m_farZ(farZ)
{ }

glm::mat4 PerspectiveCamera::as_projection_matrix() const
{
    return glm::perspective(m_fov, m_aspect, m_nearZ, m_farZ);
}

void PerspectiveCamera::set_fov(float fov)
{
    if( fov == m_fov
     || fov == 0.f )
    {
        return;
    }

    m_fov = fov;
}

void PerspectiveCamera::set_aspect(float aspect)
{
    if( m_aspect == aspect
      || aspect == 0.f)
    {
        return;
    }

    m_aspect = aspect;
}

float PerspectiveCamera::get_fov() const
{
    return m_fov;
}