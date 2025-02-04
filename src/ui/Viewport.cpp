#include "ui/Viewport.hpp"

#include "core/Project.hpp"
#include "ui/ShaderUniformPane.hpp"

Viewport::Viewport()
    : m_framebuffer{Framebuffer::create_simple(1, 1)}
    , m_camera_controller{CameraController::Type::FREECAM, glm::vec3{}}
{
}

void Viewport::render(double delta_time, ShaderUniformPane const& pane)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    if (ImGui::Begin("Viewport")) {
        if (ImGui::IsWindowFocused()) {
            m_camera_controller.update(delta_time);
        }

        auto size = ImGui::GetContentRegionAvail();
        if (size.x != m_framebuffer.width || size.y != m_framebuffer.height) {
            m_framebuffer.resize(size.x, size.y);
        }

        if (pane.draw_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        m_camera_controller.camera->draw(pane.viewing_mode, pane.uniforms, m_framebuffer, Project::get_current()->scene.value());

        ImGui::Image(m_framebuffer.color_texture, ImVec2(m_framebuffer.width, m_framebuffer.height), ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
