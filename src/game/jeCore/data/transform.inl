#include "transform.h"

namespace mtl
{

constexpr transform::transform(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation) :
	m_position(position),
	m_scale(scale),
	m_rotation(rotation)
{ }

constexpr transform::transform(const glm::vec3& position, const glm::vec3& scale, const glm::quat& rotation) :
	m_position(position),
	m_scale(scale),
	m_rotation(rotation)
{ }

constexpr glm::vec3 transform::get_position() const
{
	return m_position;
}

constexpr glm::vec3 transform::get_scale() const
{
	return m_scale;
}

glm::vec3 transform::get_rotation() const
{
	return glm::eulerAngles(m_rotation);
}

constexpr glm::quat transform::get_quat_rotation() const
{
	return m_rotation;
}

constexpr void transform::set_position(const glm::vec3& value)
{
	m_position = value;
}

constexpr void transform::set_scale(const glm::vec3& value)
{
	m_scale = value;
}

constexpr void transform::set_euler_rotation(const glm::vec3& value)
{
	m_rotation = glm::quat(value);
}

constexpr void transform::set_rotation(const glm::quat& value)
{
	m_rotation = value;
}

constexpr void transform::move(const glm::vec3& translation)
{
	m_position += translation;
}

constexpr void transform::rotate(const glm::vec3& rotation)
{
	m_rotation *= glm::quat(rotation);
}

constexpr void transform::rotate(const glm::quat& rotation)
{
	m_rotation *= rotation;
}

glm::mat4 transform::get_matrix() const
{
	return glm::translate(glm::scale(glm::mat4(1.f), m_scale) * glm::toMat4(m_rotation), m_position);
}

} // mtl