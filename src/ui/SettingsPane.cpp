#include "ui/SettingsPane.hpp"
#include "core/CameraController.hpp"
#include "core/Project.hpp"
#include "renderer/Shader.hpp"
#include <unordered_map>

void SettingsPane::render()
{
    ImGui::SetNextWindowSize(ImVec2{400, 600});
    if (ImGui::Begin("Settings")) {
        auto& config = Project::get_current()->config;

        // Draw dropdown to choose between shading modes
        std::unordered_map<ViewingMode, char const*> const shading_mode_map{
            {ViewingMode::ALBEDO, "Albedo"},
            {ViewingMode::SOLID, "Solid"},
            {ViewingMode::RENDERED, "Rendered"}};

        ImGui::SeparatorText("Shading Mode");

        if (ImGui::BeginCombo("##shading_mode", shading_mode_map.at(config.viewing_mode))) {
            for (auto const& [mode, name] : shading_mode_map) {
                if (ImGui::Selectable(name, config.viewing_mode == mode))
                    config.viewing_mode = mode;
                if (config.viewing_mode == mode)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Wireframe button
        ImGui::Checkbox("Draw wireframe", &config.draw_wireframe);

        auto fov_degrees = glm::degrees(config.fov);
        if (ImGui::SliderFloat("FOV", &fov_degrees, 10.0f, 170.0f)) {
            config.fov = glm::radians(fov_degrees);
        }

        ImGui::InputFloat("near", &config.near, 1.0f);
        ImGui::InputFloat("far", &config.far, 1.0f);

        // Lighting Controls
        ImGui::SeparatorText("Lighting Controls");
        ImGui::SliderFloat("Ambient Light Strength", &config.viewport_uniforms.ambient_strength, 0.0f, 1.0f);
        ImGui::SliderFloat("Light Strength", &config.viewport_uniforms.light.power, 0.0f, 1.0f);
        ImGui::SliderFloat("Specularity Factor", &config.viewport_uniforms.specularity_factor, 0.0f, 1.0f);
        ImGui::SliderFloat3("Light Direction", &config.viewport_uniforms.light.direction[0], .0f, 10.0f);
        ImGui::ColorEdit3("Light Color", &config.viewport_uniforms.light.color[0]);

        std::unordered_map<CameraController::Type, char const*> const camera_type_map{
            {CameraController::Type::FREECAM, "Freecam"},
            {CameraController::Type::BLENDER, "Blender"},
            {CameraController::Type::UNITY, "Unity"}};

        ImGui::SeparatorText("Camera Mode");

        if (ImGui::BeginCombo("##camera_mode", camera_type_map.at(config.camera_controller_type))) {
            for (auto const& [type, name] : camera_type_map) {
                if (ImGui::Selectable(name, config.camera_controller_type == type))
                    config.camera_controller_type = type;
                if (config.camera_controller_type == type)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SeparatorText("Camera Speed");

        ImGui::InputFloat("Movement Speed unit/s", &config.movement_speed, 100.0f);
        ImGui::InputFloat("Rotation Speed rad/s", &config.rotation_speed, 0.05f);
        ImGui::InputFloat("Zoom Speed", &config.zoom_speed, 0.1f);

        ImGui::SeparatorText("Textures");

        ImGui::ColorEdit3("Fallback Texture Color", &config.fallback_color[0]);
        ImGui::ColorEdit3("Sky Color", &config.sky_color[0]);
    }
    ImGui::End();
}
