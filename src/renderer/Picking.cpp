#include "renderer/Picking.hpp"

#include "core/Scene.hpp"
#include "renderer/Camera.hpp"

// Returns true if the iteration should stop
bool traverse_instances(InstancedNode& parent, std::function<bool(glm::mat4, InstancedNode&)> f)
{
    if (f(parent.model_matrix, parent)) {
        return true;
    }

    for (auto& child : parent.children) {
        if (traverse_instances(*child, f)) {
            return true;
        }
    }

    return false;
}

InstancedNode* Picking::get_selected_node(Camera const& camera, InstancedNode& scene, glm::vec2 cursor_position, glm::vec2 framebuffer_size)
{
    if (m_framebuffer.width != framebuffer_size.x || m_framebuffer.height != framebuffer_size.y) {
        m_framebuffer.resize(framebuffer_size.x, framebuffer_size.y);
    }

    auto const& shader = Shader::picking;

    shader.use();

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer.id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_framebuffer.width, m_framebuffer.height);

    // view/projection transformations
    shader.set_uniform(shader.uniform_locations.projection, camera.projection(m_framebuffer.aspect));
    shader.set_uniform(shader.uniform_locations.view, camera.view());

    uint32_t i = 1; // Treat 0 as error
    traverse_instances(scene, [&](auto transform_matrix, auto& instance) {
        if (!instance.node) {
            return false;
        }

        shader.set_uniform(shader.uniform_locations.id, i);
        shader.set_uniform(shader.uniform_locations.model, transform_matrix);

        for (auto const& mesh : instance.node->meshes) {
            mesh.draw();
        }

        ++i;

        return false;
    });

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer.id);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // You may wonder why we use `float` here. That's because `glReadPixels` refuses to output integers,
    // even when using flormat: GL_RED_INTEGER and type: GL_UNSIGNED_INT both here and in the frambuffer.
    // I would love to know why.
    float id = 0;
    glReadPixels(cursor_position.x, cursor_position.y, 1, 1, GL_RED, GL_FLOAT, &id);

    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    if (id == 0) {
        return nullptr;
    }

    InstancedNode* found_node{nullptr};
    i = 1;
    traverse_instances(scene, [&](auto, auto& instance) {
        if (!instance.node) {
            return false;
        }

        if (i == id) {
            found_node = &instance;
            return true;
        }
        ++i;
        return false;
    });

    return found_node;
}
