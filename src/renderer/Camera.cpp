#include "renderer/Camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 target, float fov, float far)
    : position(position)
    , target(target)
    , far(far)
    , fov(fov)
{
}

void Camera::draw(Shader& shader, Framebuffer const& framebuffer, InstancedNode const& node)
{
    shader.use();

    glViewport(0, 0, framebuffer.width, framebuffer.height);

    auto aspect = static_cast<float>(framebuffer.width) / static_cast<float>(framebuffer.height);
    glm::mat4 projection = glm::perspective(fov, aspect, 0.1f, far);
    auto view = glm::lookAt(position, target, up);

    shader.set_mat4("projection", projection);
    shader.set_mat4("view", view);

    node.traverse([&](auto transform_matrix, auto const& node) {
        for (auto const& mesh : node.meshes) {
            shader.set_mat4("model", transform_matrix);
            mesh.draw(shader);
        }
    });
}
