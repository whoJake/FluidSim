#include "Camera.h"

Camera::Camera(uint32_t width, uint32_t height, uint32_t fov) :
    m_position(),
    m_rotation(glm::quat(glm::vec3(0.f))),
    m_inverseWidth(1.f / width),
    m_inverseHeight(1.f / height),
    m_aspect(static_cast<float>(width) / height),
    m_fov(static_cast<float>(fov)),
    m_fovDistance(tan(3.14159f * 0.5f * fov / 180.f))
{ }

glm::vec3& Camera::position()
{
    return m_position;
}

glm::quat& Camera::rotation()
{
    return m_rotation;
}

void Camera::set_width(uint32_t width)
{
    m_inverseWidth = 1.f / width;
    m_aspect = static_cast<float>(width) / get_viewport_height();
}

void Camera::set_height(uint32_t height)
{
    m_inverseHeight = 1.f / height;
    m_aspect = static_cast<float>(get_viewport_width()) / height;
}

Ray Camera::get_pixel_ray(size_t x, size_t y) const
{
    float xx = (2.f * ((x * 0.5f) * m_inverseWidth) - 0.5f) * m_fovDistance * m_aspect;
    float yy = (1.f - (2.f * ((y + 0.5f) * m_inverseHeight))) * m_fovDistance;

    glm::vec3 dir = glm::normalize(glm::vec3{ xx, yy, -1.f });
    dir = glm::normalize(m_rotation * dir);

    return
    {
        m_position,
        dir
    };
}

size_t Camera::get_viewport_width() const
{
    return static_cast<size_t>(1.f / m_inverseWidth);
}

size_t Camera::get_viewport_height() const
{
    return static_cast<size_t>(1.f / m_inverseHeight);
}