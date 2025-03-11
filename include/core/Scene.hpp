#pragma once

#include "renderer/Mesh.hpp"
#include <filesystem>
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

    // Can be nullptr!
    Node const* node;

    glm::mat4 model_matrix{0.0f};
    std::vector<std::unique_ptr<InstancedNode>> children;
    std::string name;

    static unsigned int counter;
    unsigned int const id{counter++};

    void traverse(std::function<void(glm::mat4, Node const&)>) const;
    void compute_transforms(glm::mat4 = glm::mat4{1.0f});

    // This function is slow and should only be sparingly used and only when absolutely necessary.
    [[nodiscard]] InstancedNode* find_parent(InstancedNode& scene) const;
};

struct NodeLocation {
    bool has_file;
    std::filesystem::path file_path;
    std::filesystem::path node_path;

    static NodeLocation empty();
    static NodeLocation file(std::filesystem::path file_path, std::filesystem::path node_path);
};

struct Node {
    Transform transform;
    std::vector<Node> children;
    std::vector<Mesh> meshes;
    std::string name;
    NodeLocation location;

    static Node create(std::string name, Transform transform, NodeLocation location);
    [[nodiscard]] std::unique_ptr<InstancedNode> instantiate() const;
    [[nodiscard]] bool is_fully_loaded() const;
};
