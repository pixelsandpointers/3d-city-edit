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
    return deserialize_unique_ptr_instancednode(json);
}

void Serializer::serialize(Config const& source, std::ostream& target) const
{
    target << std::setw(4) << serialize(source);
}

Config Serializer::deserialize_config(std::istream& source) const
{
    auto json = nlohmann::json::parse(source);
    return deserialize_config(json);
}

nlohmann::json Serializer::serialize(InstancedNode const& source) const
{
    nlohmann::json target;
    target["name"] = source.name;
    target["position"] = source.transform.position;
    target["orientation"] = source.transform.orientation;
    target["scale"] = source.transform.scale;

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

nlohmann::json Serializer::serialize(Config const& source) const
{
    nlohmann::json target;
    target["viewing_mode"] = source.viewing_mode;
    target["viewport_uniforms"] = serialize(source.viewport_uniforms);
    target["draw_wireframe"] = source.draw_wireframe;
    target["fallback_color"] = source.fallback_color;
    target["fov"] = source.fov;
    target["near"] = source.near;
    target["far"] = source.far;
    target["sky_color"] = source.sky_color;
    target["camera_controller_type"] = source.camera_controller_type;
    target["movement_speed"] = source.movement_speed;
    target["rotation_speed"] = source.rotation_speed;
    target["zoom_speed"] = source.zoom_speed;
    target["camera_position"] = source.camera_position;
    target["camera_target"] = source.camera_target;
    target["gizmo_use_snap"] = source.gizmo_use_snap;
    target["gizmo_snap_translation"] = source.gizmo_snap_translation;
    target["gizmo_snap_rotation"] = source.gizmo_snap_rotation;
    target["gizmo_snap_scale"] = source.gizmo_snap_scale;
    return target;
}

nlohmann::json Serializer::serialize(Uniforms const& source) const
{
    nlohmann::json target;
    target["ambient_strength"] = source.ambient_strength;
    target["specularity_factor"] = source.specularity_factor;
    target["shininess"] = source.shininess;
    target["gamma"] = source.gamma;
    target["light.direction"] = source.light.direction;
    target["light.color"] = source.light.color;
    target["light.power"] = source.light.power;
    return target;
}

std::unique_ptr<InstancedNode> Serializer::deserialize_unique_ptr_instancednode(nlohmann::json& source) const
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
        children.push_back(deserialize_unique_ptr_instancednode(child));
    }

    return std::unique_ptr<InstancedNode>(new InstancedNode{
        .transform = Transform{
            .position = source["position"],
            .orientation = source["orientation"],
            .scale = source["scale"],
        },
        .node = node,
        .model_matrix = glm::mat4{},
        .children = std::move(children),
        .name = source["name"],
    });
}

Config Serializer::deserialize_config(nlohmann::json& source) const
{
    return Config{
        .viewing_mode = source["viewing_mode"],
        .viewport_uniforms = deserialize_uniforms(source["viewport_uniforms"]),
        .draw_wireframe = source["draw_wireframe"],
        .fallback_color = source["fallback_color"],
        .fov = source["fov"],
        .near = source["near"],
        .far = source["far"],
        .sky_color = source["sky_color"],
        .camera_controller_type = source["camera_controller_type"],
        .movement_speed = source["movement_speed"],
        .rotation_speed = source["rotation_speed"],
        .zoom_speed = source["zoom_speed"],
        .camera_position = source["camera_position"],
        .camera_target = source["camera_target"],
        .gizmo_use_snap = source["gizmo_use_snap"],
        .gizmo_snap_translation = source["gizmo_snap_translation"],
        .gizmo_snap_rotation = source["gizmo_snap_rotation"],
        .gizmo_snap_scale = source["gizmo_snap_scale"],
    };
}

Uniforms Serializer::deserialize_uniforms(nlohmann::json& source) const
{
    return Uniforms{
        .ambient_strength = source["ambient_strength"],
        .specularity_factor = source["specularity_factor"],
        .shininess = source["shininess"],
        .gamma = source["gamma"],
        .light = {
            .direction = source["light.direction"],
            .color = source["light.color"],
            .power = source["light.power"],
        },
    };
}

namespace glm {
    void to_json(nlohmann::json& j, vec3 const& v)
    {
        j = {v.x, v.y, v.z};
    }

    void from_json(nlohmann::json const& j, vec3& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }

    void to_json(nlohmann::json& j, vec4 const& v)
    {
        j = {v.x, v.y, v.z, v.w};
    }

    void from_json(nlohmann::json const& j, vec4& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }

    void to_json(nlohmann::json& j, quat const& q)
    {
        j = {q.x, q.y, q.z, q.w};
    }

    void from_json(nlohmann::json const& j, quat& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }
}
