#include "ui/AssetBrowser.hpp"
#include "core/Project.hpp"
#include <filesystem>
#include <imgui.h>

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
        } else {
            ImGui::TreeNodeEx(entry.path.filename().string().c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        }
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
