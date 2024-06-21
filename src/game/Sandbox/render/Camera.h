#pragma once
#include "data/ray.h"
#include "data/fixed_vector.h"

class Camera
{
public:
    Camera(uint32_t width, uint32_t height, uint32_t fov);

    glm::vec3& position();
    glm::quat get_rotation() const;

    void set_rotation(const glm::vec3& degrees);
    void rotate(const glm::vec3& degrees);

    void set_width(uint32_t width);
    void set_height(uint32_t height);
    
    mtl::ray get_pixel_ray(size_t x, size_t y) const;

    size_t get_viewport_width() const;
    size_t get_viewport_height() const;
private:
    glm::vec3 m_position;
    glm::vec3 m_rotation;

    uint32_t m_fov;
    uint32_t m_width;
    uint32_t m_height;
};