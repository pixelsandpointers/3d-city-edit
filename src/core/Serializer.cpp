#include "core/Serializer.hpp"

#include "core/Project.hpp"
#include "core/Scene.hpp"
#include "core/Serializer.hpp"
#include <glm/gtc/quaternion.hpp>

void Serializer::serialize(InstancedNode const& source, std::ostream& target) const
{
    target << std::setw(4) << serialize(source);
}

std::unique_ptr<InstancedNode> Serializer::deserialize_scene(std::istream& source) const
{
    auto json = nlohmann::json::parse(source);
    return deserialize<std::unique_ptr<InstancedNode>>(json);
}

void Serializer::serialize(Config const& source, std::ostream& target) const
{
    target << std::setw(4) << serialize(source);
}

Config Serializer::deserialize_config(std::istream& source) const
{
    auto json = nlohmann::json::parse(source);
    return deserialize<Config>(json);
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
        children.push_back(serialize(*child));
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

nlohmann::json Serializer::serialize(glm::vec4 const& source) const
{
    nlohmann::json target;
    target["x"] = source.x;
    target["y"] = source.y;
    target["z"] = source.z;
    target["w"] = source.w;
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

nlohmann::json Serializer::serialize(Config const& source) const
{
    nlohmann::json target;
    target["viewing_mode"] = source.viewing_mode;
    target["viewport_uniforms"] = serialize(source.viewport_uniforms);
    target["draw_wireframe"] = source.draw_wireframe;
    target["fallback_color"] = serialize(source.fallback_color);
    target["fov"] = source.fov;
    target["camera_controller_type"] = source.camera_controller_type;
    target["movement_speed"] = source.movement_speed;
    target["rotation_speed"] = source.rotation_speed;
    target["zoom_speed"] = source.zoom_speed;
    target["camera_position"] = serialize(source.camera_position);
    target["camera_target"] = serialize(source.camera_target);
    return target;
}

nlohmann::json Serializer::serialize(Uniforms const& source) const
{
    nlohmann::json target;
    target["ambient_strength"] = source.ambient_strength;
    target["specularity_factor"] = source.specularity_factor;
    target["shininess"] = source.shininess;
    target["gamma"] = source.gamma;
    target["light.direction"] = serialize(source.light.direction);
    target["light.color"] = serialize(source.light.color);
    target["light.power"] = source.light.power;
    return target;
}

template <>
std::unique_ptr<InstancedNode> Serializer::deserialize(nlohmann::json& source) const
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

    auto children = std::vector<std::unique_ptr<InstancedNode>>{};
    for (auto& child : source["children"]) {
        children.push_back(deserialize<std::unique_ptr<InstancedNode>>(child));
    }

    return std::unique_ptr<InstancedNode>(new InstancedNode{
        .transform = Transform{
            .position = deserialize<glm::vec3>(source["position"]),
            .orientation = deserialize<glm::quat>(source["orientation"]),
            .scale = deserialize<glm::vec3>(source["scale"]),
        },
        .node = node,
        .model_matrix = glm::mat4{},
        .children = std::move(children),
        .name = source["name"],
    });
}

template <>
glm::vec3 Serializer::deserialize(nlohmann::json& source) const
{
    return glm::vec3{source["x"], source["y"], source["z"]};
}

template <>
glm::vec4 Serializer::deserialize(nlohmann::json& source) const
{
    return glm::vec4{source["x"], source["y"], source["z"], source["w"]};
}

template <>
glm::quat Serializer::deserialize(nlohmann::json& source) const
{
    return glm::quat{source["w"], source["x"], source["y"], source["z"]};
}

template <>
Config Serializer::deserialize(nlohmann::json& source) const
{
    return Config{
        .viewing_mode = source["viewing_mode"],
        .viewport_uniforms = deserialize<Uniforms>(source["viewport_uniforms"]),
        .draw_wireframe = source["draw_wireframe"],
        .fallback_color = deserialize<glm::vec3>(source["fallback_color"]),
        .fov = source["fov"],
        .camera_controller_type = source["camera_controller_type"],
        .movement_speed = source["movement_speed"],
        .rotation_speed = source["rotation_speed"],
        .zoom_speed = source["zoom_speed"],
        .camera_position = deserialize<glm::vec3>(source["camera_position"]),
        .camera_target = deserialize<glm::vec3>(source["camera_target"]),
    };
}

template <>
Uniforms Serializer::deserialize(nlohmann::json& source) const
{
    return Uniforms{
        .ambient_strength = source["ambient_strength"],
        .specularity_factor = source["specularity_factor"],
        .shininess = source["shininess"],
        .gamma = source["gamma"],
        .light = {
            .direction = deserialize<glm::vec4>(source["light.direction"]),
            .color = deserialize<glm::vec3>(source["light.color"]),
            .power = source["light.power"],
        },
    };
}
