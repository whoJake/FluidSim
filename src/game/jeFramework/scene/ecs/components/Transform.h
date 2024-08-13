#pragma once

#include "../Component.h"

namespace fw
{
namespace ecs
{

class Transform : public Component
{
public:
    Transform();
    ~Transform() = default;

    DEFAULT_COPY(Transform);
    DEFAULT_MOVE(Transform);

    glm::vec3& position();
    const glm::vec3& position() const;

    glm::vec3& scale();
    const glm::vec3& scale() const;

    glm::vec3& rotation();
    const glm::vec3& rotation() const;
private:
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_rotation;
};

} // ecs
} // fw