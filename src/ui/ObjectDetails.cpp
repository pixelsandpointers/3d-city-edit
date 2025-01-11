#include "ui/ObjectDetails.hpp"
#include <imgui.h>
#include <string>

ObjectDetails::ObjectDetails() = default;

char object_label[128] = {""};
float rotation[3][128] = {{0}, {0}, {0}};
float scale[3][128] = {{1}, {1}, {1}};
float position[3][128] = {{0}, {0}, {0}};
bool toggle_shading = false;

void ObjectDetails::render()
{
    ImVec2 min_size(300, 150); // Base minimum size
    ImVec2 max_size(FLT_MAX, FLT_MAX); // No maximum size limit

    // Set the child window position to the right side of the parent window
    ImVec2 child_pos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(child_pos);

    // Apply size constraint before creating the window
    ImGui::SetNextWindowSizeConstraints(min_size, max_size);
    if (ImGui::Begin("Object Details", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {

        ImGui::InputText("Label", object_label, IM_ARRAYSIZE(object_label));

        char const* rotation_labels[] = {"X##rotation0", "Y##rotation1", "Z##rotation2"};
        char const* scale_labels[] = {"X##scale0", "Y##scale1", "Z##scale2"};
        char const* position_labels[] = {"X##position0", "Y##position1", "Z##position2"};

        // Rotation Row
        ImGui::Text("Rotation");
        for (int i = 0; i < 3; i++) {
            ImGui::PushItemWidth(100); // Set width for uniformity
            ImGui::InputFloat(rotation_labels[i], rotation[i], 0.1f, 1.0f, "%.3f");
            ImGui::PopItemWidth();
            if (i < 2)
                ImGui::SameLine(); // Keep on same row
        }

        // Scale Row
        ImGui::Text("Scale");
        for (int i = 0; i < 3; i++) {
            ImGui::PushItemWidth(100);
            ImGui::InputFloat(scale_labels[i], scale[i], 0.1f, 1.0f, "%.3f");
            ImGui::PopItemWidth();
            if (i < 2)
                ImGui::SameLine();
        }

        // Position Row
        ImGui::Text("Position");
        for (int i = 0; i < 3; i++) {
            ImGui::PushItemWidth(100);
            ImGui::InputFloat(position_labels[i], position[i], 0.1f, 1.0f, "%.3f");
            ImGui::PopItemWidth();
            if (i < 2)
                ImGui::SameLine();
        }

        if (ImGui::Checkbox("Shading", &toggle_shading)) {
            // add callback
        }
    }

    ImGui::End();
}
