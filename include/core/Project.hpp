#pragma once

#include "core/Config.hpp"
#include "core/Scene.hpp"
#include "renderer/Texture.hpp"
#include <filesystem>
#include <unordered_map>

// Needed when using glm::vec4 as key in std::unordered_map
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

struct FSCacheNode {
    enum class Type {
        DIRECTORY,
        MODEL,
        TEXTURE,
        UNITY_MATERIAL,
        UNITY_META,
        OTHER,
    };

    std::filesystem::path path;
    std::filesystem::file_time_type mtime;
    Type type;
    std::vector<FSCacheNode> children;

    FSCacheNode* get_child(std::filesystem::path);
};

class Project {
public:
    std::filesystem::path root;
    InstancedNode* selected_node{nullptr};
    std::unique_ptr<InstancedNode> scene;
    Config config;

    static Project* get_current();
    static Project* load(std::filesystem::path);
    void store();

    FSCacheNode* get_fs_cache();
    FSCacheNode* get_fs_cache(std::filesystem::path);
    std::optional<std::filesystem::path> get_fs_cache_from_guid(std::string const&) const;
    Texture const* get_texture(std::filesystem::path);
    Node* get_model(std::filesystem::path);
    Node* get_cached_model(std::filesystem::path);
    Node* get_node(NodeLocation);
    void update(double current_time);
    Texture const* fallback_texture() const;
    Texture const* white_texture() const;

private:
    friend struct Performance;
    static std::unique_ptr<Project> current;

    ColorTexture m_fallback_texture{ColorTexture::single_color(glm::vec4{1.0f})};
    ColorTexture m_white_texture{ColorTexture::single_color(glm::vec4{1.0f})};
    std::unordered_map<std::filesystem::path, Texture> m_textures;
    std::unordered_map<std::filesystem::path, Node> m_models;
    std::unique_ptr<FSCacheNode> m_fs_cache;
    double m_fs_cache_last_updated{0};
    std::unordered_map<std::string, std::filesystem::path> m_guid_mappings;

    Project(std::filesystem::path);
    void rebuild_fs_cache();
    void rebuild_fs_cache_helper(FSCacheNode&);
};
