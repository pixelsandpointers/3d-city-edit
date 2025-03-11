#include "ui/ObjectDetails.hpp"
#include "core/Project.hpp"
#include "ui/Viewport.hpp"
#include <imgui.h>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

ObjectDetails::ObjectDetails() = default;

InstancedNode* reset_instance_full(InstancedNode* node)
{
    auto project = Project::get_current();
    auto base_node = node->node;
    auto parent = node->find_parent(*project->scene);

    if (!base_node || !parent) {
        return node;
    }

    auto& children = parent->children;
    auto position = std::find_if(children.begin(), children.end(), [&](std::unique_ptr<InstancedNode> const& current) {
        return current.get() == node;
    });
    if (position == children.end()) {
        return node;
    }

    auto new_node = base_node->instantiate();
    node = project->selected_node = new_node.get();
    position = children.insert(position, std::move(new_node));
    children.erase(position + 1);
    project->scene->compute_transforms();

    return node;
}

void reset_instance_transform(InstancedNode* node)
{
    auto project = Project::get_current();
    auto base_node = node->node;
    if (!base_node) {
        return;
    }

    node->transform = base_node->transform;
    project->scene->compute_transforms();
}

char object_label[128] = {""};

void ObjectDetails::render(CameraController& camera_controller)
{
    auto transform_changed = false;
    auto project = Project::get_current();
    auto node = project->selected_node;

    if (ImGui::Begin("Object Details", nullptr)) {
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

        // Misc
        if (ImGui::BeginPopup("reset_mode")) {
            if (ImGui::Button("transform only")) {
                reset_instance_transform(node);
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::Button("transform and children")) {
                node = reset_instance_full(node);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::Button("Reset")) {
            ImGui::OpenPopup("reset_mode");
        }

        ImGui::SameLine();
        if (ImGui::Button("Focus Camera [F]")) {
            camera_controller.focus_on(*project->selected_node);
        }
    }

    if (project->scene && transform_changed) {
        project->scene->compute_transforms();
    }

    ImGui::End();
}
