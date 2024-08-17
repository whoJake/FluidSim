#pragma once

namespace mtl
{

class transform
{
public:
	constexpr transform(const glm::vec3& position = { },
			  const glm::vec3& scale = { 1.f, 1.f, 1.f },
		      const glm::vec3& rotation = { });

	constexpr transform(const glm::vec3& position,
			  const glm::vec3& scale,
			  const glm::quat& rotation);

	constexpr ~transform() = default;

	constexpr glm::vec3 get_position() const;
	constexpr glm::vec3 get_scale() const;
	inline glm::vec3 get_rotation() const;
	constexpr glm::quat get_quat_rotation() const;

	constexpr void set_position(const glm::vec3& value);
	constexpr void set_scale(const glm::vec3& value);
	constexpr void set_euler_rotation(const glm::vec3& value);
	constexpr void set_rotation(const glm::quat& value);

	constexpr void move(const glm::vec3& translation);
	constexpr void rotate(const glm::vec3& rotation);
	constexpr void rotate(const glm::quat& rotation);

	inline glm::mat4 get_matrix() const;
	inline glm::mat4 get_matrix_as_view() const;
	// constexpr void set_matrix(const glm::mat4& value);
private:
	// TODO: condense this all into a matrix
	glm::vec3 m_position;
	glm::vec3 m_scale;
	glm::quat m_rotation;
};

} // mtl

#ifndef INC_TRANSFORM_INL
#define INC_TRANSFORM_INL
#include "transform.inl"
#endif