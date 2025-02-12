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
    InstancedNode deserialize_scene(std::istream&) const;
    void serialize(Config const&, std::ostream&) const;
    Config deserialize_config(std::istream&) const;

private:
    Project& m_project;

    [[nodiscard]] nlohmann::json serialize(InstancedNode const&) const;
    [[nodiscard]] nlohmann::json serialize(glm::vec3 const&) const;
    [[nodiscard]] nlohmann::json serialize(glm::vec4 const&) const;
    [[nodiscard]] nlohmann::json serialize(glm::quat const&) const;
    [[nodiscard]] nlohmann::json serialize(Config const&) const;
    [[nodiscard]] nlohmann::json serialize(Uniforms const&) const;

    template <class T>
    T deserialize(nlohmann::json&) const;
    template <>
    InstancedNode deserialize(nlohmann::json& source) const;
    template <>
    glm::vec3 deserialize(nlohmann::json& source) const;
    template <>
    glm::vec4 deserialize(nlohmann::json& source) const;
    template <>
    glm::quat deserialize(nlohmann::json& source) const;
    template <>
    Config deserialize(nlohmann::json& source) const;
    template <>
    Uniforms deserialize(nlohmann::json& source) const;
};
