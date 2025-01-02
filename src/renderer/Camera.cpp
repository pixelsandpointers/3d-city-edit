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
    Uniforms const& uniforms,
    Framebuffer const& framebuffer,
    InstancedNode const& node)
{
    // depending on the viewing mode allow wireframe, flat or lid shading
    shader.use();

    glViewport(0, 0, framebuffer.width, framebuffer.height);

    glm::mat4 projection = glm::perspective(fov, framebuffer.aspect, near, far);
    auto view = glm::lookAt(position, target, up);

    // view/projection transformations
    shader.set_mat4("projection", projection);
    shader.set_mat4("view", view);
    shader.set_vec3("cameraPos", position);
    shader.set_float("ambientStrength", uniforms.ambient_strength);
    shader.set_float("specularityFactor", uniforms.specularity_factor);
    shader.set_float("shininess", uniforms.shininess);
    shader.set_float("gamma", uniforms.gamma);
    shader.set_vec3("light.direction", glm::vec3(uniforms.light.direction));
    shader.set_vec3("light.color", uniforms.light.color);
    shader.set_float("light.power", uniforms.light.power);

    node.traverse([&](auto transform_matrix, auto const& node_data) {
        for (auto const& mesh : node_data.meshes) {
            shader.set_mat4("model", transform_matrix);
            mesh.draw(shader);
        }
    });
}
