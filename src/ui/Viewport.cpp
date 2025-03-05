#include "ui/Viewport.hpp"

#include "core/Project.hpp"

// clang-format off
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
// clang-format on

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

Viewport::Viewport()
    : m_framebuffer{Framebuffer::create_simple(1, 1)}
    , m_camera_controller{CameraController::Type::UNITY, glm::vec3{}}
{
}

void Viewport::render(double delta_time)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    if (ImGui::Begin("Viewport")) {
        auto project = Project::get_current();
        auto& config = project->config;

        m_camera_controller.camera->position = config.camera_position;
        m_camera_controller.camera->target = config.camera_target;

        if (ImGui::IsWindowFocused()) {
            m_camera_controller.update(delta_time, ImGui::IsWindowHovered());
        }

        auto size = ImGui::GetContentRegionAvail();
        if (size.x != m_framebuffer.width || size.y != m_framebuffer.height) {
            m_framebuffer.resize(size.x, size.y);
        }

        if (config.draw_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        m_camera_controller.camera->fov = config.fov;
        m_camera_controller.type = config.camera_controller_type;
        m_camera_controller.movement_speed = config.movement_speed;
        m_camera_controller.rotation_speed = config.rotation_speed;
        m_camera_controller.zoom_speed = config.zoom_speed;
        m_camera_controller.camera->draw(config.viewing_mode, config.viewport_uniforms, m_framebuffer, *project->scene);
        if (project->selected_node) {
            m_camera_controller.camera->draw_outline(m_framebuffer, *project->selected_node);
        }

        config.camera_position = m_camera_controller.camera->position;
        config.camera_target = m_camera_controller.camera->target;

        ImGui::Image(m_framebuffer.color_texture, ImVec2(m_framebuffer.width, m_framebuffer.height), ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            auto const mouse_pos = ImGui::GetMousePos();
            auto const window_pos = ImGui::GetWindowPos();
            // Some awful workaround for not being able to get the position of the `ImGui::Image` directly. At least I have no clue how to do that.
            // The 19 pixels are for the window titlebar.
            auto const relative_pos = glm::vec2{mouse_pos.x - window_pos.x, m_framebuffer.height - mouse_pos.y - window_pos.y + 19};

            project->selected_node = m_picker.get_selected_node(*m_camera_controller.camera, *project->scene, relative_pos, glm::vec2{m_framebuffer.width, m_framebuffer.height});
        }

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_framebuffer.width, m_framebuffer.height);

        if (project->selected_node) {
            auto const view = m_camera_controller.camera->view();
            auto const projection = m_camera_controller.camera->projection(m_framebuffer.aspect);
            auto model_matrix = project->selected_node->model_matrix;
            auto delta_matrix = glm::mat4{1.0f};
            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(model_matrix), glm::value_ptr(delta_matrix))) {
                auto& transform = project->selected_node->transform;
                glm::vec3 delta_scale;
                glm::quat delta_orientation;
                glm::vec3 delta_position;
                glm::vec3 delta_skew_unused;
                glm::vec4 delta_projection_unused;
                glm::decompose(delta_matrix, delta_scale, delta_orientation, delta_position, delta_skew_unused, delta_projection_unused);
                transform.position += delta_position;
                project->scene->compute_transforms();
            }
        }
    }

    // Focus Viewport window not only with the left, but also the middle and right mouse button.
    if (!ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
        ImGui::FocusWindow(ImGui::GetCurrentWindow());
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
