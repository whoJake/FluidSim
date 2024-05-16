#pragma once

class Camera
{
public:
    virtual ~Camera() { };

    void rotate(const glm::vec3& eulerRotation);
    void rotate(const glm::quat& rotation);

    void translate(const glm::vec3& translation);

    glm::vec3 get_position() const;

    glm::vec3 get_euler_rotation() const;

    glm::quat get_rotation() const;

    virtual glm::mat4 as_projection_matrix() const = 0;

    glm::mat4 as_view_matrix() const;
protected:
    Camera();
protected:
    glm::vec3 m_position;
    glm::quat m_rotation;
};

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera(float fov, float aspect, float nearZ, float farZ);
    ~PerspectiveCamera() { };

    glm::mat4 as_projection_matrix() const override;

    void set_fov(float fov);

    void set_aspect(float aspect);

    float get_fov() const;
private:
    float m_fov;
    float m_aspect;
    float m_nearZ;
    float m_farZ;
};