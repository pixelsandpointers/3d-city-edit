#include "ui/AssetBrowser.hpp"
#include "core/Project.hpp"
#include <filesystem>
#include <imgui.h>
#include <iostream>

constexpr auto imgui_color_red = ImVec4{1.0f, 0.0f, 0.0f, 1.0f};
constexpr auto imgui_color_grey = ImVec4{0.7f, 0.7f, 0.7f, 1.0f};
constexpr auto imgui_color_light_grey = ImVec4{0.3f, 0.3f, 0.3f, 1.0f};

void traverse_model(Node const& node)
{
    ImGui::PushStyleColor(ImGuiCol_Text, imgui_color_grey);
    for (auto const& child : node.children) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (!child.children.empty()) {
            bool open = ImGui::TreeNodeEx(child.name.c_str());
            if (open) {
                traverse_model(child);
                ImGui::TreePop();
            }
            continue;
        }

        ImGui::TreeNodeEx(child.name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
    }
    ImGui::PopStyleColor(1);
}

void traverse_directory(FSCacheNode const& node)
{
    for (auto const& entry : node.children) {
        bool is_folder = entry.type == FSCacheNode::Type::DIRECTORY;
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (is_folder) {
            bool open = ImGui::TreeNodeEx(entry.path.filename().string().c_str());
            if (open) {
                traverse_directory(entry);
                ImGui::TreePop();
            }
            continue;
        }

        if (entry.type == FSCacheNode::Type::MODEL) {
            bool open = ImGui::TreeNodeEx(entry.path.filename().string().c_str());
            if (open) {
                auto model = Project::get_current()->get_model(entry.path);
                if (!model) {
                    ImGui::PushStyleColor(ImGuiCol_Text, imgui_color_red);
                    ImGui::TreeNodeEx(entry.path.filename().string().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                    ImGui::PopStyleColor(1);
                    continue;
                }

                traverse_model(*model);
                ImGui::TreePop();
            }
            continue;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, imgui_color_light_grey);
        ImGui::TreeNodeEx(entry.path.filename().string().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        ImGui::PopStyleColor(1);
    }
}

void AssetBrowser::render()
{
    ImGui::Begin("Asset Browser");
    if (ImGui::BeginTable("assetbrowser_directory_tree", 1)) {
        traverse_directory(*Project::get_current()->get_fs_cache());
        ImGui::EndTable();
    }
    ImGui::End();
}
