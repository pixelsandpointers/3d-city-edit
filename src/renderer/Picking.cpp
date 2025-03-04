#include "renderer/Picking.hpp"
#include <iostream>

void select_object(Camera* camera, InstancedNode const& scene_root, glm::vec2 window_size, glm::vec2 cursor_pos)
{
    // Convert screen coordinates to Normalized Device Coordinates (NDC)
    float ndc_x = (2.0f * cursor_pos.x) / window_size.x - 1.0f;
    float ndc_y = 1.0f - (2.0f * cursor_pos.y) / window_size.y;

    glm::vec4 ray_clip(ndc_x, ndc_y, -1.0f, 1.0f);

    // Convert to View Space
    glm::mat4 projection = glm::perspective(camera->fov,
        window_size.x / static_cast<float>(window_size.y),
        camera->near,
        camera->far);

    glm::vec4 ray_view = glm::inverse(projection) * ray_clip;
    ray_view.z = -1.0f; // Set direction to -1 (forward in view space)
    ray_view.w = 0.0f; // Directional vector, not a point

    // Convert to World Space
    glm::mat4 view_matrix = glm::lookAt(camera->position, camera->target, camera->up);
    glm::vec3 ray_world = glm::vec3(glm::inverse(view_matrix) * ray_view);
    ray_world = glm::normalize(ray_world); // Normalize the ray direction

    // Test for intersection with objects in the scene
    float closest_distance = std::numeric_limits<float>::max();
    Node* closest_object = nullptr;

    // TODO: Add some function `InstancedNode::traverse_instances` that can get `InstancedNode&`
    scene_root.traverse([&](glm::mat4 world_transform, Node const& node) {
        auto distance = test_intersection(camera->position, ray_world, node, world_transform);
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_object = const_cast<Node*>(&node);
        }
    });

    // Handle the selection
    if (closest_object) {
        std::cout << "Selected object: " << closest_object->name << std::endl;
        // Additional actions like highlighting or UI updates can be performed here
    }
}

float test_intersection(glm::vec3 const& ray_origin, glm::vec3 const& ray_dir, Node const& object, glm::mat4 const& world_transform)
{
    // Compute inverse ray direction for AABB intersection
    glm::vec3 inv_dir = 1.0f / ray_dir;

    auto min_distance = std::numeric_limits<float>::max();

    // Loop through meshes in the Node
    for (auto const& mesh : object.meshes) {
        auto min_bound = glm::vec3(world_transform * glm::vec4(mesh.aabb.min, 1));
        auto max_bound = glm::vec3(world_transform * glm::vec4(mesh.aabb.max, 1));
        auto tmin = std::numeric_limits<float>::lowest();
        auto tmax = std::numeric_limits<float>::max();

        bool intersects = true;

        // Perform intersection tests for each axis
        for (int i = 0; i < 3; ++i) {
            float t1 = (min_bound[i] - ray_origin[i]) * inv_dir[i];
            float t2 = (max_bound[i] - ray_origin[i]) * inv_dir[i];

            // This works, but also detects backwards matches
            tmin = std::max(tmin, std::min(t1, t2));
            tmax = std::min(tmax, std::max(t1, t2));

            if (tmin > tmax) {
                intersects = false;
                break;
            }
        }

        if (intersects && tmin > 0.0f && tmin < min_distance) {
            min_distance = tmin;
        }
    }
    return min_distance;
}
