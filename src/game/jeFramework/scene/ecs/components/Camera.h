#pragma once

#include "../Component.h"

namespace fw
{
namespace ecs
{

class Camera3d : public Component
{
public:

    f32& fov();
    const f32& fov() const;

    f32& near();
    const f32& near() const;

    f32& far();
    const f32& far() const;

    glm::vec2& near_far();
    const glm::vec2& near_far() const;

    glm::mat4 calculate_matrix() const;
private:
    f32 m_fov;
    glm::vec2 m_planes;
};

} // ecs
} // fw