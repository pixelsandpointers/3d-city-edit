#include "ui/AssetBrowser.hpp"
#include <filesystem>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>
#include <iostream>

constexpr auto imgui_color_grey = ImVec4{0.7f, 0.7f, 0.7f, 1.0f};
constexpr auto imgui_color_light_grey = ImVec4{0.3f, 0.3f, 0.3f, 1.0f};
constexpr auto imgui_treenode_leaf_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;

bool imgui_treenodeex_with_color(char const* label, int flags, ImVec4 color)
{
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    auto open = ImGui::TreeNodeEx(label, flags);
    ImGui::PopStyleColor(1);
    return open;
}

AssetBrowser::AssetBrowser()
{
    glGenFramebuffers(1, &m_model_preview_framebuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_model_preview_framebuffer.id);

    glGenTextures(1, &m_model_preview_texture);
    glBindTexture(GL_TEXTURE_2D, m_model_preview_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_model_preview_framebuffer.width, m_model_preview_framebuffer.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_model_preview_texture, 0);

    glGenRenderbuffers(1, &m_model_preview_depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_model_preview_depth_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_model_preview_framebuffer.width, m_model_preview_framebuffer.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_model_preview_depth_rbo);

    auto framebuffer_ready = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (!framebuffer_ready) {
        std::cerr << "ui/AssetBrowser: Framebuffer is not complete!\n";
        glDeleteFramebuffers(1, &m_model_preview_framebuffer.id);
        m_model_preview_framebuffer.id = 0;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AssetBrowser::traverse_model(Node const& node)
{
    ImGui::PushStyleColor(ImGuiCol_Text, imgui_color_grey);
    for (auto const& child : node.children) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        int flags = is_selected_item_equal(&child) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
        bool open = false;

        if (!child.children.empty()) {
            open = ImGui::TreeNodeEx(child.name.c_str(), flags);
        } else {
            ImGui::TreeNodeEx(child.name.c_str(), flags | imgui_treenode_leaf_flags);
        }

        if (ImGui::IsItemClicked()) {
            m_selected_item = &child;
            m_preview_dirty = true;
        }

        if (open) {
            traverse_model(child);
            ImGui::TreePop();
        }
    }
    ImGui::PopStyleColor(1);
}

void AssetBrowser::traverse_directory(FSCacheNode const& node)
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
            auto model = Project::get_current()->get_cached_model(entry.path);
            int flags = is_selected_item_equal(model) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
            bool open = ImGui::TreeNodeEx(entry.path.filename().string().c_str(), flags);

            if (ImGui::IsItemClicked()) {
                if (!model) {
                    model = Project::get_current()->get_model(entry.path);
                }
                m_selected_item = model;
                m_preview_dirty = true;
            }

            if (open) {
                if (!model) {
                    model = Project::get_current()->get_model(entry.path);
                }
                if (model) {
                    traverse_model(*model);
                }
                ImGui::TreePop();
            }
            continue;
        }

        auto color = imgui_color_light_grey;
        if (entry.type == FSCacheNode::Type::TEXTURE) {
            color = imgui_color_grey;
        }
        int flags = is_selected_item_equal(entry.path) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
        imgui_treenodeex_with_color(entry.path.filename().string().c_str(), imgui_treenode_leaf_flags | flags, color);

        if (ImGui::IsItemClicked()) {
            m_selected_item = entry.path;
            m_preview_dirty = true;
        }
    }
}

void AssetBrowser::render()
{
    ImGui::Begin("Asset Browser");
    auto available_region = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("tree", ImVec2{available_region.x * 0.75f, available_region.y}, ImGuiChildFlags_ResizeX);
    if (ImGui::BeginTable("assetbrowser_directory_tree", 1)) {
        traverse_directory(*Project::get_current()->get_fs_cache());
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("preview");
    prepare_preview();
    ImGui::Text("%s", m_preview_name.c_str());
    if (m_preview_texture) {
        ImGui::Image(m_preview_texture, ImVec2{static_cast<float>(m_model_preview_framebuffer.width), static_cast<float>(m_model_preview_framebuffer.height)}, ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});
    } else {
        ImGui::Text("Preview placeholder");
    }

    if (m_selected_item.has_value()) {
        if (auto* value = std::get_if<Node const*>(&m_selected_item.value()); value != nullptr) {
            auto node = *value;
            ImGui::Text("Model node");
            ImGui::Text("Meshes: %zu", node->meshes.size());
            ImGui::Text("Direct children: %zu", node->children.size());
        }
        if (auto* value = std::get_if<std::filesystem::path>(&m_selected_item.value()); value != nullptr) {
            auto node = Project::get_current()->get_fs_cache(*value);
            if (node && node->type == FSCacheNode::Type::TEXTURE) {
                auto texture = Project::get_current()->get_texture(*value);
                if (texture) {
                    ImGui::Text("Filetype: ? Image"); // TODO: Get image format?
                    ImGui::Text("Size: %d x %d", texture->width, texture->height);
                    ImGui::Text("Channels: %d", texture->channels);
                }
            }
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void AssetBrowser::prepare_preview()
{
    if (!m_preview_dirty) {
        return;
    }

    m_preview_dirty = false;
    m_preview_texture = 0;

    if (!m_selected_item.has_value()) {
        return;
    }

    if (auto* value = std::get_if<Node const*>(&m_selected_item.value()); value != nullptr) {
        m_preview_name = (*value)->name;
        render_model_preview();
        m_preview_texture = m_model_preview_texture;
        return;
    }

    if (auto* value = std::get_if<std::filesystem::path>(&m_selected_item.value()); value != nullptr) {
        m_preview_name = value->filename().string();
        auto node = Project::get_current()->get_fs_cache(*value);
        if (node && node->type == FSCacheNode::Type::TEXTURE) {
            auto texture = Project::get_current()->get_texture(*value);
            m_preview_texture = texture->m_id;
        }
    }
}

void AssetBrowser::render_model_preview()
{
    if (!m_selected_item.has_value() || !std::holds_alternative<Node const*>(m_selected_item.value()) || m_model_preview_framebuffer.id == 0) {
        return;
    }

    auto selected_node = std::get<Node const*>(m_selected_item.value());

    // Compute AABB
    AABB aabb{
        .min = glm::vec3{std::numeric_limits<float>::max()},
        .max = glm::vec3{-std::numeric_limits<float>::max()},
    };

    auto instance = selected_node->instanciate();
    instance.compute_transforms();

    instance.traverse([&](glm::mat4 transform_matrix, Node const& node) {
        for (auto const& mesh : node.meshes) {
            aabb = aabb.merge(AABB{
                .min = transform_matrix * glm::vec4{mesh.aabb.min, 1.0f},
                .max = transform_matrix * glm::vec4{mesh.aabb.max, 1.0f},
            });
        }
    });

    // Render
    glBindFramebuffer(GL_FRAMEBUFFER, m_model_preview_framebuffer.id);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    auto camera_offset = (aabb.min - aabb.max) * glm::vec3{0.5f};
    m_model_preview_camera.fov = glm::radians(45.0f);
    m_model_preview_camera.near = 0.1f;
    m_model_preview_camera.far = 100000.0f;
    m_model_preview_camera.position = aabb.max - camera_offset;
    m_model_preview_camera.target = aabb.min;
    m_model_preview_camera.draw(m_model_preview_shader, m_model_preview_uniforms, m_model_preview_framebuffer, instance);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool AssetBrowser::is_selected_item_equal(NodeVariantType to_compare)
{
    if (!m_selected_item.has_value()) {
        return false;
    }

    if (std::holds_alternative<Node const*>(m_selected_item.value()) && std::holds_alternative<Node const*>(to_compare)) {
        return std::get<Node const*>(m_selected_item.value()) == std::get<Node const*>(to_compare);
    }

    if (std::holds_alternative<std::filesystem::path>(m_selected_item.value()) && std::holds_alternative<std::filesystem::path>(to_compare)) {
        return std::get<std::filesystem::path>(m_selected_item.value()) == std::get<std::filesystem::path>(to_compare);
    }

    return false;
}
