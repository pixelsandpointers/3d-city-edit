#include "ui/ObjectDetails.hpp"
#include "core/Project.hpp"
#include <imgui.h>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

ObjectDetails::ObjectDetails() = default;

char object_label[128] = {""};
bool toggle_shading = false;

void ObjectDetails::render()
{
    ImVec2 min_size(300, 150); // Base minimum size
    ImVec2 max_size(FLT_MAX, FLT_MAX); // No maximum size limit

    // Set the child window position to the right side of the parent window
    ImVec2 child_pos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(child_pos);

    auto transform_changed = false;
    auto project = Project::get_current();
    auto node = project->selected_node;

    // Apply size constraint before creating the window
    ImGui::SetNextWindowSizeConstraints(min_size, max_size);
    if (ImGui::Begin("Object Details", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        if (!node) {
            ImGui::Text("No node selected");
            ImGui::End();
            return;
        }

        std::strcpy(object_label, node->name.c_str());
        if (ImGui::InputText("Label", object_label, IM_ARRAYSIZE(object_label))) {
            node->name = object_label;
        }

        char const* rotation_labels[] = {"X##rotation0", "Y##rotation1", "Z##rotation2"};
        char const* scale_labels[] = {"X##scale0", "Y##scale1", "Z##scale2"};
        char const* position_labels[] = {"X##position0", "Y##position1", "Z##position2"};

        // Rotation Row
        auto rotation = glm::degrees(glm::eulerAngles(node->transform.orientation));
        ImGui::Text("Rotation");
        for (int i = 0; i < 3; i++) {
            ImGui::PushItemWidth(100); // Set width for uniformity
            if (ImGui::InputFloat(rotation_labels[i], &rotation[i], 0.1f, 1.0f, "%.3f")) {
                auto euler_rad = glm::radians(rotation);
                node->transform.orientation = glm::eulerAngleXYZ(euler_rad.x, euler_rad.y, euler_rad.z);
                transform_changed = true;
            }
            ImGui::PopItemWidth();
            if (i < 2)
                ImGui::SameLine(); // Keep on same row
        }

        // Scale Row
        ImGui::Text("Scale");
        for (int i = 0; i < 3; i++) {
            ImGui::PushItemWidth(100);
            if (ImGui::InputFloat(scale_labels[i], &node->transform.scale[i], 1.0f, 10.0f, "%.3f")) {
                transform_changed = true;
            }
            ImGui::PopItemWidth();
            if (i < 2)
                ImGui::SameLine();
        }

        // Position Row
        ImGui::Text("Position");
        for (int i = 0; i < 3; i++) {
            ImGui::PushItemWidth(100);
            if (ImGui::InputFloat(position_labels[i], &node->transform.position[i], 1.0f, 10.0f, "%.3f")) {
                transform_changed = true;
            }
            ImGui::PopItemWidth();
            if (i < 2)
                ImGui::SameLine();
        }
    }

    if (project->scene && transform_changed) {
        project->scene->compute_transforms();
    }

    ImGui::End();
}
