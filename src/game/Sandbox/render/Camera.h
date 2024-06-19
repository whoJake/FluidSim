#pragma once
#include "data/ray.h"
#include "data/fixed_vector.h"

class Camera
{
public:
    Camera(uint32_t width, uint32_t height, uint32_t fov);

    glm::vec3& position();
    glm::quat& rotation();

    void set_width(uint32_t width);
    void set_height(uint32_t height);
    
    mtl::ray get_pixel_ray(size_t x, size_t y) const;

    size_t get_viewport_width() const;
    size_t get_viewport_height() const;
private:
    glm::vec3 m_position;
    glm::quat m_rotation;

    float m_inverseWidth;
    float m_inverseHeight;
    float m_aspect;
    float m_fov;
    float m_fovDistance;
};