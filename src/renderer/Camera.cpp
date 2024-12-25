#include "renderer/Camera.hpp"

#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 target, float fov)
    : position(position)
    , target(target)
    , fov(fov)
{
}

// add viewing type => lid, wireframe, etc
void Camera::draw(Shader& shader,
    std::unordered_map<std::string, std::variant<int, float, bool, glm::vec2, glm::vec3, glm::vec4, glm::mat2, glm::mat3, glm::mat4>> const& uniforms,
    Framebuffer const& framebuffer,
    InstancedNode const& node)
{
    // depending on the viewing mode allow wireframe, flat or lid shading
    shader.use();

    glViewport(0, 0, framebuffer.width, framebuffer.height);

    glm::mat4 projection = glm::perspective(fov, framebuffer.aspect, near, far);
    auto view = glm::lookAt(position, target, up);
    auto light_pos = std::get<glm::vec4>(uniforms.at("light.direction"));

    // view/projection transformations
    shader.set_mat4("projection", projection);
    shader.set_mat4("view", view);
    shader.set_vec3("cameraPos", position);
    shader.set_bool("useBlinn", std::get<bool>(uniforms.at("useBlinn")));
    shader.set_float("ambientStrength", std::get<float>(uniforms.at("ambientStrength")));
    shader.set_vec3("light.color", std::get<glm::vec3>(uniforms.at("light.color")));

    node.traverse([&](auto transform_matrix, auto const& node_data) {
        for (auto const& mesh : node_data.meshes) {
            shader.set_mat4("model", transform_matrix);
            // set light position relative to transformation matrix of the model
            shader.set_vec3("light.direction", glm::vec3{transform_matrix * light_pos});
            mesh.draw(shader);
        }
    });
}
