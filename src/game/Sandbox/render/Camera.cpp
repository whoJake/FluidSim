#include "Camera.h"

Camera::Camera(uint32_t width, uint32_t height, uint32_t fov) :
    m_position(),
    m_rotation(glm::vec3(0.f)),
    m_fov(fov),
    m_width(width),
    m_height(height)
{ }

glm::vec3& Camera::position()
{
    return m_position;
}

glm::quat Camera::get_rotation() const
{
     return glm::angleAxis(glm::radians(m_rotation.z), glm::vec3(0.f, 0.f, 1.f))
          * glm::angleAxis(glm::radians(m_rotation.y), glm::vec3(0.f, 1.f, 0.f))
          * glm::angleAxis(glm::radians(m_rotation.x), glm::vec3(1.f, 0.f, 0.f));
}

void Camera::set_rotation(const glm::vec3& degrees)
{
    m_rotation = degrees;
}

void Camera::rotate(const glm::vec3& degrees)
{
    m_rotation += degrees;
    if( m_rotation.x > 360.f )
        m_rotation.x -= 360.f;
    if( m_rotation.x < 0.f )
        m_rotation.x += 360.f;

    if( m_rotation.y > 360.f )
        m_rotation.y -= 360.f;
    if( m_rotation.y < 0.f )
        m_rotation.y += 360.f;

    if( m_rotation.z > 360.f )
        m_rotation.z -= 360.f;
    if( m_rotation.z < 0.f )
        m_rotation.z += 360.f;
}

void Camera::set_width(uint32_t width)
{
    m_width = width;
}

void Camera::set_height(uint32_t height)
{
    m_height = height;
}

mtl::ray Camera::get_pixel_ray(size_t x, size_t y) const
{
    // get uv's
    glm::vec2 uv
    {
        x / static_cast<float>(m_width),
        y / static_cast<float>(m_height)
    };

    // transform into -1, +1 range
    uv = (uv * 2.f) - glm::vec2(1.f, 1.f);
    uv.y *= -1; // flip y axis

    float aspect = m_width / static_cast<float>(m_height);
    // shrink y to correct for aspect ratio
    uv.y /= aspect;

    float cameraDistance = 1.f / (std::tanf(glm::radians(m_fov * 0.5f)));

    glm::vec3 direction = glm::normalize(get_rotation() * glm::vec3(uv, cameraDistance));
    return
    {
        m_position,
        // direction
        direction,
        // inverse direction
        glm::vec3(1.f / direction.x, 1.f / direction.y, 1.f / direction.z)
    };
}

size_t Camera::get_viewport_width() const
{
    return m_width;
}

size_t Camera::get_viewport_height() const
{
    return m_height;
}