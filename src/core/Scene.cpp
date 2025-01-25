#include "core/Scene.hpp"

#include <glm/ext/matrix_transform.hpp>

glm::mat4 Transform::get_local_matrix() const
{
    auto matrix = glm::mat4{1.0f};
    matrix = glm::translate(matrix, position);
    matrix = matrix * glm::mat4_cast(orientation);
    matrix = glm::scale(matrix, scale);
    return matrix;
}

[[nodiscard]] glm::vec3 Transform::orientation_euler() const
{
    return glm::eulerAngles(orientation);
}

void Transform::set_orientation_euler(glm::vec3 euler)
{
    orientation = glm::quat{euler};
}

Node Node::create(std::string name, Transform transform, NodeLocation location)
{
    return Node{
        .transform = transform,
        .children = {},
        .meshes = {},
        .name = name,
        .location = location,
    };
}

InstancedNode Node::instanciate() const
{
    InstancedNode new_node{
        .transform = transform,
        .node = this,
        .model_matrix = glm::mat4{},
        .children = {},
        .name = name,
    };

    for (auto const& child : children) {
        new_node.children.push_back(child.instanciate());
    }

    return new_node;
}

[[nodiscard]] bool Node::is_fully_loaded() const
{
    for (auto const& mesh : meshes) {
        if (!mesh.is_fully_loaded()) {
            return false;
        }
    }

    for (auto const& child : children) {
        if (!child.is_fully_loaded()) {
            return false;
        }
    }

    return true;
}

void InstancedNode::traverse(std::function<void(glm::mat4, Node const&)> f) const
{
    if (node) {
        f(model_matrix, *node);
    }

    for (auto const& child : children) {
        child.traverse(f);
    }
}

void InstancedNode::compute_transforms(glm::mat4 parent_transform)
{
    model_matrix = parent_transform * transform.get_local_matrix();
    for (auto& child : children) {
        child.compute_transforms(model_matrix);
    }
}

NodeLocation NodeLocation::empty()
{
    return NodeLocation{
        .has_file = false,
        .file_path = "",
        .node_path = "",
    };
}

NodeLocation NodeLocation::file(std::filesystem::path file_path, std::filesystem::path node_path)
{
    return NodeLocation{
        .has_file = true,
        .file_path = file_path,
        .node_path = node_path,
    };
}
