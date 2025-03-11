#pragma once

#include "core/Config.hpp"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

struct InstancedNode;
class Project;

class Serializer {
public:
    Serializer(Project& project)
        : m_project{project}
    { }

    void serialize(InstancedNode const&, std::ostream&) const;
    std::unique_ptr<InstancedNode> deserialize_scene(std::istream&) const;
    void serialize(Config const&, std::ostream&) const;
    Config deserialize_config(std::istream&) const;

private:
    Project& m_project;

    [[nodiscard]] nlohmann::json serialize(InstancedNode const&) const;
    [[nodiscard]] nlohmann::json serialize(Config const&) const;
    [[nodiscard]] nlohmann::json serialize(Uniforms const&) const;

    std::unique_ptr<InstancedNode> deserialize_unique_ptr_instancednode(nlohmann::json& source) const;
    Config deserialize_config(nlohmann::json& source) const;
    Uniforms deserialize_uniforms(nlohmann::json& source) const;
};

namespace glm {
    void to_json(nlohmann::json& j, vec3 const& v);
    void from_json(nlohmann::json const& j, vec3& v);
    void to_json(nlohmann::json& j, vec4 const& v);
    void from_json(nlohmann::json const& j, vec4& v);
    void to_json(nlohmann::json& j, quat const& q);
    void from_json(nlohmann::json const& j, quat& v);
}
