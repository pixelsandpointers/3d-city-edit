#include "core/Serializer.hpp"

#include "core/Project.hpp"
#include "core/Scene.hpp"
#include "core/Serializer.hpp"
#include <glm/gtc/quaternion.hpp>

void Serializer::serialize(InstancedNode const& source, std::ostream& target) const
{
    target << std::setw(4) << serialize(source);
}

InstancedNode Serializer::deserialize_scene(std::istream& source) const
{
    auto json = nlohmann::json::parse(source);
    return deserialize<InstancedNode>(json);
}

nlohmann::json Serializer::serialize(InstancedNode const& source) const
{
    nlohmann::json target;
    target["name"] = source.name;
    target["position"] = serialize(source.transform.position);
    target["orientation"] = serialize(source.transform.orientation);
    target["scale"] = serialize(source.transform.scale);

    auto has_file = source.node && source.node->location.has_file;
    target["has_file"] = has_file;
    if (has_file) {
        auto project_root = Project::get_current()->root;
        target["file_path"] = std::filesystem::relative(source.node->location.file_path, project_root);
        target["node_path"] = source.node->location.node_path;
    }

    auto children = nlohmann::json::array();
    for (auto const& child : source.children) {
        children.push_back(serialize(child));
    }
    target["children"] = children;
    return target;
}

nlohmann::json Serializer::serialize(glm::vec3 const& source) const
{
    nlohmann::json target;
    target["x"] = source.x;
    target["y"] = source.y;
    target["z"] = source.z;
    return target;
}

nlohmann::json Serializer::serialize(glm::quat const& source) const
{
    nlohmann::json target;
    target["x"] = source.x;
    target["y"] = source.y;
    target["z"] = source.z;
    target["w"] = source.w;
    return target;
}

template <>
InstancedNode Serializer::deserialize(nlohmann::json& source) const
{
    auto location = NodeLocation::empty();
    location.has_file = source["has_file"];
    if (location.has_file) {
        auto project_root = Project::get_current()->root;
        location.file_path = project_root / static_cast<std::filesystem::path>(static_cast<std::string>(source["file_path"])); // This doesn't look particularly nice
        location.node_path = static_cast<std::string>(source["node_path"]);
    }

    auto node = location.has_file
        ? m_project.get_node(location)
        : nullptr;

    auto children = std::vector<InstancedNode>{};
    for (auto& child : source["children"]) {
        children.push_back(deserialize<InstancedNode>(child));
    }

    return InstancedNode{
        .transform = Transform{
            .position = deserialize<glm::vec3>(source["position"]),
            .orientation = deserialize<glm::quat>(source["orientation"]),
            .scale = deserialize<glm::vec3>(source["scale"]),
        },
        .node = node,
        .model_matrix = glm::mat4{},
        .children = children,
        .name = source["name"],
    };
}

template <>
glm::vec3 Serializer::deserialize(nlohmann::json& source) const
{
    return glm::vec3{source["x"], source["y"], source["z"]};
}

template <>
glm::quat Serializer::deserialize(nlohmann::json& source) const
{
    return glm::quat{source["w"], source["x"], source["y"], source["z"]};
}
