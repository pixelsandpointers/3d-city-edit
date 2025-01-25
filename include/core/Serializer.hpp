#pragma once

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

private:
    Project& m_project;

    [[nodiscard]] nlohmann::json serialize(InstancedNode const&) const;
    [[nodiscard]] nlohmann::json serialize(glm::vec3 const&) const;
    [[nodiscard]] nlohmann::json serialize(glm::quat const&) const;

    template <class T>
    T deserialize(nlohmann::json&) const;
    template <>
    InstancedNode deserialize(nlohmann::json& source) const;
    template <>
    glm::vec3 deserialize(nlohmann::json& source) const;
    template <>
    glm::quat deserialize(nlohmann::json& source) const;
};
