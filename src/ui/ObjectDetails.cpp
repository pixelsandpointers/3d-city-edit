#include "ui/ObjectDetails.hpp"
#include <imgui.h>
#include <string>

ObjectDetails::ObjectDetails(){}

char object_label[128] = {""};

char rotation[3][128] = {{"0"}, {"0"}, {"0"}};

char scale[3][128] = { {"1"}, {"1"}, {"1"}};

char position[3][128] = { {"0"}, {"0"}, {"0"} };

bool toggle_shading = false;

void ObjectDetails::render() {
    ImVec2 min_size(300, 150);  // Base minimum size
    ImVec2 max_size(FLT_MAX, FLT_MAX);  // No maximum size limit
    const char* axis_labels[3] = { "X", "Y", "Z" };

    // Set the child window position to the right side of the parent window
    ImVec2 child_pos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(child_pos);

    // Apply size constraint before creating the window
    ImGui::SetNextWindowSizeConstraints(min_size, max_size);
    ImGui::Begin("Object Details", nullptr, ImGuiWindowFlags_AlwaysAutoResize |ImGuiWindowFlags_NoMove);

    ImGui::InputText("Label", object_label, IM_ARRAYSIZE(object_label));

    // Rotation Row
    ImGui::Text("Rotation");
    for (int i = 0; i < 3; i++) {
        ImGui::PushItemWidth(80);  // Set width for uniformity
        ImGui::InputText((std::string(axis_labels[i]) + "##rotation" + std::to_string(i)).c_str(), rotation[i], IM_ARRAYSIZE(rotation[i]));
        ImGui::PopItemWidth();
        if (i < 2) ImGui::SameLine();  // Keep on same row
    }

    // Scale Row
    ImGui::Text("Scale");
    for (int i = 0; i < 3; i++) {
        ImGui::PushItemWidth(80);
        ImGui::InputText((std::string(axis_labels[i]) + "##scale" + std::to_string(i)).c_str(), rotation[i], IM_ARRAYSIZE(rotation[i]));
        ImGui::PopItemWidth();
        if (i < 2) ImGui::SameLine();
    }

    // Position Row
    ImGui::Text("Position");
    for (int i = 0; i < 3; i++) {
        ImGui::PushItemWidth(80);
        ImGui::InputText((std::string(axis_labels[i]) + "##position" + std::to_string(i)).c_str(), rotation[i], IM_ARRAYSIZE(rotation[i]));
        ImGui::PopItemWidth();
        if (i < 2) ImGui::SameLine();
    }

    if (ImGui::Checkbox("Shading", &toggle_shading)) {
        // add callback

    }
    ImGui::End();
}