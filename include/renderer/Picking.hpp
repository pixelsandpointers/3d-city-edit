#pragma once

#include "core/Input.hpp"
#include "core/Project.hpp"
#include "renderer/Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Function to handle object selection
void select_object(Camera* camera, InstancedNode const& scene_root);

// Example helper test_intersection function (using the AABB)
bool test_intersection(glm::vec3 const& ray_origin, glm::vec3 const& ray_dir, Node const& object, glm::mat4 const& world_transform);
