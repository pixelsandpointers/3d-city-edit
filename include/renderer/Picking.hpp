#pragma once

#include "renderer/Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Function to handle object selection
void select_object(Camera* camera, InstancedNode const& scene_root, glm::vec2 window_size, glm::vec2 cursor_pos);

// Example helper test_intersection function (using the AABB)
float test_intersection(glm::vec3 const& ray_origin, glm::vec3 const& ray_dir, Node const& object, glm::mat4 const& world_transform);
