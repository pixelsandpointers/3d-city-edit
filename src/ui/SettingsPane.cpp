#include "ui/SettingsPane.hpp"
#include "core/CameraController.hpp"

void SettingsPane::render()
{
    ImGui::SetNextWindowSize(ImVec2{400, 600});
    if (ImGui::Begin("Settings")) {
        // Draw dropdown to choose between shading modes
        std::unordered_map<ViewingMode, char const*> const shading_mode_map{
            {ViewingMode::ALBEDO, "Albedo"},
            {ViewingMode::SOLID, "Solid"},
            {ViewingMode::RENDERED, "Rendered"}};

        ImGui::SeparatorText("Shading Mode");

        if (ImGui::BeginCombo("##shading_mode", shading_mode_map.at(viewing_mode))) {
            for (auto const& [mode, name] : shading_mode_map) {
                if (ImGui::Selectable(name, viewing_mode == mode))
                    viewing_mode = mode;
                if (viewing_mode == mode)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Wireframe button
        ImGui::Checkbox("Draw wireframe", &draw_wireframe);

        // Lighting Controls
        ImGui::SeparatorText("Lighting Controls");
        ImGui::SliderFloat("Ambient Light Strength", &uniforms.ambient_strength, 0.0f, 1.0f);
        ImGui::SliderFloat("Light Strength", &uniforms.light.power, 0.0f, 1.0f);
        ImGui::SliderFloat("Specularity Factor", &uniforms.specularity_factor, 0.0f, 1.0f);
        ImGui::SliderFloat3("Light Direction", &uniforms.light.direction[0], .0f, 10.0f);
        ImGui::ColorEdit3("Light Color", &uniforms.light.color[0]);

        std::unordered_map<CameraController::Type, char const*> const camera_type_map{
            {CameraController::Type::FREECAM, "Freecam"},
            {CameraController::Type::BLENDER, "Blender"},
            {CameraController::Type::UNITY, "Unity"}};

        ImGui::SeparatorText("Camera Mode");

        if (ImGui::BeginCombo("##camera_mode", camera_type_map.at(camera_type))) {
            for (auto const& [type, name] : camera_type_map) {
                if (ImGui::Selectable(name, camera_type == type))
                    camera_type = type;
                if (camera_type == type)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SeparatorText("Camera Speed");

        ImGui::InputFloat("Movement Speed unit/s", &movement_speed, 100.0f);
        ImGui::InputFloat("Rotation Speed rad/s", &rotation_speed, 0.05f);
        ImGui::InputFloat("Zoom Speed", &zoom_speed, 0.1f);

        ImGui::SeparatorText("Textures");

        ImGui::ColorEdit3("Fallback Texture Color", &fallback_color[0]);
        ImGui::End();
    }
}
