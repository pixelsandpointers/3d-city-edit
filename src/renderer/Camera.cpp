#include "renderer/Camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 target, float fov)
    : position(position)
    , target(target)
    , fov(fov)
{
}

void Camera::draw(Shader& shader, Framebuffer const& framebuffer, InstancedNode const& node)
{
    shader.use();

    glViewport(0, 0, framebuffer.width, framebuffer.height);

    glm::mat4 projection = glm::perspective(fov, framebuffer.aspect, near, far);
    auto view = glm::lookAt(position, target, up);
    auto light_position = glm::vec4{1.f};

    // view/projection transformations
    shader.set_mat4("projection", projection);
    shader.set_mat4("view", view);
    shader.set_vec3("cameraPos", position);
    shader.set_bool("useBlinn", true);
    shader.set_float("ambientStrength", 0.1f);
    shader.set_vec3("light.color", glm::vec3{0.7f, 0.4f, 0.1f});

    node.traverse([&](auto transform_matrix, auto const& node_data) {
        for (auto const& mesh : node_data.meshes) {
            shader.set_mat4("model", transform_matrix);
            // set light position relative to transformation matrix of the model
            shader.set_vec3("light.direction", glm::vec3{transform_matrix * light_position});
            mesh.draw(shader);
        }
    });
}
