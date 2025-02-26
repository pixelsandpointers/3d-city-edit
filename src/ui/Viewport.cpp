#include "ui/Viewport.hpp"

#include "core/Project.hpp"
#include <imgui_internal.h>

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
            m_camera_controller.update(delta_time);
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
        m_camera_controller.type = config.camera_controller_type;
        m_camera_controller.movement_speed = config.movement_speed;
        m_camera_controller.rotation_speed = config.rotation_speed;
        m_camera_controller.zoom_speed = config.zoom_speed;
        m_camera_controller.camera->draw(config.viewing_mode, config.viewport_uniforms, m_framebuffer, *project->scene);

        config.camera_position = m_camera_controller.camera->position;
        config.camera_target = m_camera_controller.camera->target;

        ImGui::Image(m_framebuffer.color_texture, ImVec2(m_framebuffer.width, m_framebuffer.height), ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});
    }

    // Focus Viewport window not only with the left, but also the middle and right mouse button.
    if (!ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && (ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
        ImGui::FocusWindow(ImGui::GetCurrentWindow());
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
