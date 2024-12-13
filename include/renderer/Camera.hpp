#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
  Camera(glm::vec3 position, float yaw = -90.f, float pitch = 0.f, float fov = 45.f);
  [[nodiscard]] glm::mat4 get_view_matrix() const;
  [[nodiscard]] glm::mat4 get_projection_matrix(float aspect, float near, float far) const;

public:
  glm::vec3 m_position;
  float m_yaw, m_pitch, m_fov;
  const float m_sensitivity = 0.1f;
};