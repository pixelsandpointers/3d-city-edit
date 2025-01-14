#pragma once

#include "renderer/Mesh.hpp"
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

class Mesh;
struct Node;

struct Transform {
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;

    [[nodiscard]] glm::mat4 get_local_matrix() const;
    [[nodiscard]] glm::vec3 orientation_euler() const;
    void set_orientation_euler(glm::vec3);
};

// The model matrix needs to be cached because the performance would be awful otherwise.
// However, caching the matrix in the `Node` itself would result in issues when the node is rendered multiple times.
// To be able to change the transform of each instances separately, InstancedNode needs its own transform component.
struct InstancedNode {
    Transform transform;
    Node const* node;
    glm::mat4 model_matrix{0.0f};
    std::vector<InstancedNode> children;
    std::string name;

    void traverse(std::function<void(glm::mat4, Node const&)>) const;
    void compute_transforms(glm::mat4 = glm::mat4{1.0f});
};

struct Node {
    Transform transform;
    std::vector<Node> children;
    std::vector<Mesh> meshes;
    std::string name;

    static Node create(std::string name, Transform t);
    [[nodiscard]] InstancedNode instanciate() const;
};
