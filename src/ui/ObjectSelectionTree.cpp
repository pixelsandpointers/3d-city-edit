#include "ui/ObjectSelectionTree.hpp"
#include "imgui.h"

constexpr auto imgui_treenode_leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;

ObjectSelectionTree::ObjectSelectionTree(){}



void ObjectSelectionTree::traverse_nodes(InstancedNode const& root){
    for (auto const& child : root.children) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        bool open = false;
        //int flags = is_selected_item_equal(&child) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;

        if (!child.children.empty()) {
            open = ImGui::TreeNodeEx(child.node->name.c_str());
        } else {
            ImGui::TreeNodeEx(child.node->name.c_str(), imgui_treenode_leaf_flags);
        }
/*
        if (ImGui::IsItemClicked()) {
            m_selected_item = &child;
            m_preview_dirty = true;
        }*/

        if (open) {
            traverse_nodes(child);
            ImGui::TreePop();
        }
    }

}

void ObjectSelectionTree::render(InstancedNode const& root){
    ImGui::Begin("Object Tree");

    ImGui::BeginTable("table0", 1);
    traverse_nodes(root);

    ImGui::EndTable();
    ImGui::End();
}

