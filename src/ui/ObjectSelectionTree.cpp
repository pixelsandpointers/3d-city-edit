#include "ui/ObjectSelectionTree.hpp"
#include "core/Project.hpp"
#include <imgui.h>
#include <imgui_internal.h>

constexpr auto imgui_treenode_leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;

ObjectSelectionTree::ObjectSelectionTree() = default;

std::unique_ptr<InstancedNode> instantiate_and_rename_node(Node const& node)
{
    auto new_node = node.instantiate();
    // Ugly hack to rename only the root node to the model filename
    if (node.location.has_file && node.location.node_path == "/" + node.name) {
        new_node->name = node.location.file_path.filename().string();
    }

    return new_node;
}

void ObjectSelectionTree::traverse_nodes(InstancedNode& root)
{
    auto* project = Project::get_current();

    for (auto child_it = root.children.begin(); child_it != root.children.end(); ++child_it) {
        auto& child = *child_it;

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        bool open = false;

        auto is_selected = child.get() == project->selected_node;

        if (ImGui::IsWindowFocused() && is_selected && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
            child_it = root.children.erase(child_it) - 1;
            project->selected_node = nullptr;
            continue;
        }

        auto flags_selected = is_selected
            ? ImGuiTreeNodeFlags_Selected
            : ImGuiTreeNodeFlags_None;

        auto const treenode_label = child->name + "##node_" + std::to_string(m_imgui_treenode_id++);

        if (!child->children.empty()) {
            open = ImGui::TreeNodeEx(treenode_label.c_str(), flags_selected);
        } else {
            ImGui::TreeNodeEx(treenode_label.c_str(), imgui_treenode_leaf_flags | flags_selected);
        }

        // Drop node onto TreeNode -> instantiate as child
        if (ImGui::BeginDragDropTarget()) {
            if (auto payload = ImGui::AcceptDragDropPayload("node")) {
                auto node_to_instantiate = *static_cast<Node const**>(payload->Data);
                child->children.push_back(instantiate_and_rename_node(*node_to_instantiate));
                project->scene->compute_transforms();
            }
            ImGui::EndDragDropTarget();
        }

        // Drop between TreeNodes
        auto const current_rect = GImGui->LastItemData.Rect;
        if (m_prev_rect.has_value()) {
            auto const left_edge = std::min(current_rect.Min.x, m_prev_rect->Min.x);
            auto const right_edge = std::max(current_rect.Max.x, m_prev_rect->Max.x);
            auto const top_edge = m_prev_rect->Max.y;
            auto const between_rect = ImRect{ImVec2{left_edge, top_edge}, ImVec2{right_edge, current_rect.Min.y}};

            if (ImGui::BeginDragDropTargetCustom(between_rect, GImGui->LastItemData.ID)) {
                // Drop node between TreeNodes -> instantiate as sibling
                if (auto payload = ImGui::AcceptDragDropPayload("node")) {
                    auto node_to_instantiate = *static_cast<Node const**>(payload->Data);
                    child_it = root.children.insert(child_it, instantiate_and_rename_node(*node_to_instantiate));
                    project->scene->compute_transforms();
                }

                ImGui::EndDragDropTarget();
            }
        }
        m_prev_rect = current_rect;

        if (ImGui::IsItemClicked()) {
            project->selected_node = child.get();
        }

        if (open) {
            traverse_nodes(*child);
            ImGui::TreePop();
        }
    }
}

void ObjectSelectionTree::render()
{
    if (ImGui::Begin("Object Tree")) {
        if (auto& scene = Project::get_current()->scene; scene) {
            if (ImGui::BeginTable("table0", 1)) {
                m_prev_rect = {};
                m_imgui_treenode_id = 0;
                traverse_nodes(*scene);
            }
            ImGui::EndTable();
        } else {
            ImGui::Text("No scene open");
        }
    }

    ImGui::End();
}
