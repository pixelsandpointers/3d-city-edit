#include "ui/ShaderUniformPane.hpp"

void ShaderUniformPane::render()
{
    ImGui::Begin("Shading and Lighting Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Draw dropdown to choose between shading modes
    std::unordered_map<ViewingMode, char const*> const shading_mode_map{
        {ViewingMode::ALBEDO, "Albedo"},
        {ViewingMode::SOLID, "Solid"},
        {ViewingMode::RENDERED, "Rendered"}};

    ImGui::SeparatorText("Shading Mode");

    if (ImGui::BeginCombo("", shading_mode_map.at(viewing_mode))) {
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
    ImGui::SliderFloat("Light Strength", &uniforms.light.power, 0.0f, 100.0f);
    ImGui::SliderFloat3("Light Direction", &uniforms.light.direction[0], .0f, 10.0f);
    ImGui::ColorEdit3("Light Color", &uniforms.light.color[0]);

    ImGui::End();
}
