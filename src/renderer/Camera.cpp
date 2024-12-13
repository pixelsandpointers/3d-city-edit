#include "renderer/Camera.hpp"


Camera::Camera(glm::vec3 position, float yaw, float pitch, float fov)
    : m_position(position), m_yaw(yaw), m_pitch(pitch), m_fov(fov)
{
}

glm::mat4 Camera::get_view_matrix() const
{
    glm::vec3 direction{
        cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
        sin(glm::radians(m_pitch)),
        sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))};

    direction = glm::normalize(direction);

    glm::vec3 right = glm::normalize(glm::cross(direction, {0.0f, 1.0f, 0.0f}));
    glm::vec3 up = glm::cross(right, direction);

    return glm::lookAt(m_position, m_position + direction, up);
}

glm::mat4 Camera::get_projection_matrix(float const aspect, float const near, float const far) const
{
    return glm::perspective(glm::radians(m_fov), aspect, near, far);
}

