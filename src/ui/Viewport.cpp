#include "ui/Viewport.hpp"
#include "core/Input.hpp"
#include "core/Project.hpp"
#include <map>

// clang-format off
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
// clang-format on

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

Viewport::Viewport()
    : m_framebuffer{Framebuffer::create_simple(1, 1, Framebuffer::Preset::RGB_UNSIGNED_INTEGRAL_NORMALIZED, 4)}
    , m_blitted_framebuffer{Framebuffer::create_simple(1, 1, Framebuffer::Preset::RGB_UNSIGNED_INTEGRAL_NORMALIZED, 0)}
    , m_camera_controller{CameraController::Type::UNITY, glm::vec3{}}
{
}

std::map<Viewport::GizmoOperation, char const*> const gizmo_operation_names{
    {Viewport::GizmoOperation::TRANSLATE, "Translate"},
    {Viewport::GizmoOperation::ROTATE, "Rotate"},
    {Viewport::GizmoOperation::SCALE, "Scale"},
};

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
            m_blitted_framebuffer.resize(size.x, size.y);
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

        if (m_framebuffer.num_samples > 0) {
            m_framebuffer.blit(m_blitted_framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        config.camera_position = m_camera_controller.camera->position;
        config.camera_target = m_camera_controller.camera->target;

        if (m_framebuffer.num_samples > 0) {
            ImGui::Image(m_blitted_framebuffer.color_texture, ImVec2(m_framebuffer.width, m_framebuffer.height), ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});
        } else {
            ImGui::Image(m_framebuffer.color_texture, ImVec2(m_framebuffer.width, m_framebuffer.height), ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});
        }

        if (ImGui::IsWindowHovered() && !ImGuizmo::IsOver() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
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
            // Keyboard shortcuts to set gizmo operation
            if (Input::key_pressed(GLFW_KEY_E)) {
                gizmo_operation = GizmoOperation::SCALE;
            } else if (Input::key_pressed(GLFW_KEY_Q)) {
                gizmo_operation = GizmoOperation::TRANSLATE;
            } else if (Input::key_pressed(GLFW_KEY_R)) {
                gizmo_operation = GizmoOperation::ROTATE;
            }

            ImGuizmo::OPERATION operation;
            glm::vec3 snap_size;
            switch (gizmo_operation) {
            case GizmoOperation::TRANSLATE:
                operation = ImGuizmo::TRANSLATE;
                snap_size = glm::vec3{config.gizmo_snap_translation};
                break;
            case GizmoOperation::ROTATE:
                operation = ImGuizmo::ROTATE;
                snap_size = glm::vec3{config.gizmo_snap_rotation};
                break;
            case GizmoOperation::SCALE:
                operation = ImGuizmo::SCALE;
                snap_size = glm::vec3{config.gizmo_snap_scale};
                break;
            }

            auto const view = m_camera_controller.camera->view();
            auto const projection = m_camera_controller.camera->projection(m_framebuffer.aspect);
            auto model_matrix = project->selected_node->model_matrix;
            auto delta_matrix = glm::mat4{1.0f};
            if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), operation, ImGuizmo::WORLD, glm::value_ptr(model_matrix), glm::value_ptr(delta_matrix), config.gizmo_use_snap ? glm::value_ptr(snap_size) : nullptr)) {
                auto& transform = project->selected_node->transform;
                glm::vec3 delta_scale;
                glm::quat delta_orientation;
                glm::vec3 delta_position;
                glm::vec3 delta_skew_unused;
                glm::vec4 delta_projection_unused;
                glm::decompose(delta_matrix, delta_scale, delta_orientation, delta_position, delta_skew_unused, delta_projection_unused);
                switch (gizmo_operation) {
                case GizmoOperation::TRANSLATE:
                    transform.position += delta_position;
                    break;
                case GizmoOperation::ROTATE:
                    transform.orientation = glm::normalize(delta_orientation) * transform.orientation;
                    break;
                case GizmoOperation::SCALE:
                    transform.scale *= delta_scale;
                    auto const min_scale = 0.1f;
                    if (transform.scale.x < min_scale) {
                        transform.scale.x = min_scale;
                    }
                    if (transform.scale.y < min_scale) {
                        transform.scale.y = min_scale;
                    }
                    if (transform.scale.z < min_scale) {
                        transform.scale.z = min_scale;
                    }
                    break;
                }
                project->scene->compute_transforms();
            }

            auto const window_pos = ImGui::GetWindowPos();
            ImGui::SetNextWindowPos(ImVec2{window_pos.x + 10, window_pos.y + 30});
            ImGui::SetNextWindowSize(ImVec2{200, 100});
            if (ImGui::BeginChild("gizmo_settings_window")) {
                if (ImGui::BeginCombo("##gizmo_mode_combo", gizmo_operation_names.at(gizmo_operation))) {
                    for (auto const& [operation, name] : gizmo_operation_names) {
                        if (ImGui::Selectable(name, gizmo_operation == operation))
                            gizmo_operation = operation;
                        if (gizmo_operation == operation)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::Checkbox("Use snapping", &config.gizmo_use_snap);

                float* snap_size;
                float step;
                float fast_step;
                switch (gizmo_operation) {
                case GizmoOperation::TRANSLATE:
                    snap_size = &config.gizmo_snap_translation;
                    step = 10.0f;
                    fast_step = 100.0f;
                    break;
                case GizmoOperation::ROTATE:
                    snap_size = &config.gizmo_snap_rotation;
                    step = 1.0f;
                    fast_step = 10.0f;
                    break;
                case GizmoOperation::SCALE:
                    snap_size = &config.gizmo_snap_scale;
                    step = 0.1f;
                    fast_step = 1.0f;
                    break;
                }
                ImGui::InputFloat("snap size", snap_size, step, fast_step);
            }
            ImGui::EndChild();
        }
    }

    // Focus Viewport window not only with the left, but also the middle and right mouse button.
    if (!ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
        ImGui::FocusWindow(ImGui::GetCurrentWindow());
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
