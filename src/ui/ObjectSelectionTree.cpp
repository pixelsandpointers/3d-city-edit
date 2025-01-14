#include "ui/ObjectSelectionTree.hpp"
#include "core/Project.hpp"
#include "imgui.h"

constexpr auto imgui_treenode_leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;

ObjectSelectionTree::ObjectSelectionTree() = default;

void ObjectSelectionTree::traverse_nodes(InstancedNode& root)
{
    auto* project = Project::get_current();

    for (auto& child : root.children) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        bool open = false;

        auto flags_selected = &child == project->selected_node
            ? ImGuiTreeNodeFlags_Selected
            : ImGuiTreeNodeFlags_None;

        if (!child.children.empty()) {
            open = ImGui::TreeNodeEx(child.name.c_str(), flags_selected);
        } else {
            ImGui::TreeNodeEx(child.name.c_str(), imgui_treenode_leaf_flags | flags_selected);
        }

        if (ImGui::IsItemClicked()) {
            project->selected_node = &child;
        }

        if (open) {
            traverse_nodes(child);
            ImGui::TreePop();
        }
    }
}

void ObjectSelectionTree::render()
{
    if (ImGui::Begin("Object Tree")) {
        if (auto& scene = Project::get_current()->scene; scene.has_value()) {
            if (ImGui::BeginTable("table0", 1)) {
                traverse_nodes(scene.value());
            }
            ImGui::EndTable();
        } else {
            ImGui::Text("No scene open");
        }
    }

    ImGui::End();
}
