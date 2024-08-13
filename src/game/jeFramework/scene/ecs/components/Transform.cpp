#include "Transform.h"

namespace fw
{
namespace ecs
{

Transform::Transform() :
    m_position(0.f),
    m_scale(1.f),
    m_rotation(0.f)
{ }

glm::vec3& Transform::position()
{
    return m_position;
}

const glm::vec3& Transform::position() const
{
    return m_position;
}

glm::vec3& Transform::scale()
{
    return m_scale;
}

const glm::vec3& Transform::scale() const
{
    return m_scale;
}

glm::vec3& Transform::rotation()
{
    return m_rotation;
}

const glm::vec3& Transform::rotation() const
{
    return m_rotation;
}

} // ecs
} // fw