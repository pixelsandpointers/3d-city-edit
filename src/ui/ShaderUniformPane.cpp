#include "ui/ShaderUniformPane.hpp"

void ShaderUniformPane::render() {
    ImGui::Begin("Shading and Lighting Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Draw dropdown to choose between shading modes
    const std::unordered_map<const char *, ViewingMode> shading_mode_map{
            {"Albedo",   ViewingMode::ALBEDO},
            {"Solid",    ViewingMode::SOLID},
            {"Rendered", ViewingMode::RENDERED},
    };
    static const char *current_mode = "Rendered";

    ImGui::SeparatorText("Shading Mode");

    if (ImGui::BeginCombo("", current_mode)) {
        for (const auto &[name, mode]: shading_mode_map) {
            if (ImGui::Selectable(name, name == current_mode))
                current_mode = name;
            if (name == current_mode)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    // TODO: expose current ViewingMode to Uniforms
    viewing_mode = shading_mode_map.at(current_mode);


    // Wireframe button
    if (ImGui::Button("Toggle Wireframe")) {
        draw_wireframe = !draw_wireframe;
    }

    // TODO: Lighting controls - enable change of position, color and ambient strength
    ImGui::SeparatorText("Lighting Controls");
    ImGui::SliderFloat("Ambient Light", &uniforms.ambient_strength, 0.0f, 1.0f);
    ImGui::SliderFloat3("Light Direction", &uniforms.light.direction[0], .0f, 100.0f);
    ImGui::ColorEdit3("Light Color", &uniforms.light.color[0]);

    ImGui::End();
}
